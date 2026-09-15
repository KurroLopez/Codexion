*Este proyecto ha sido creado comoparte del currículo de 42 por fralopez*

# Descripcion

`codexion` es un programa en C, escrito sobre `pthread`, en que hay **programadores ("coders")** que necesitan **dos "dongles"** (llaves USB de seguridad compartidas con sus vecinos) para poder **compilar**.

El objetivo es simular concurrencia real con hilos: evitar interbloqueos (*deadlock*) e inanición (*starvation*), gestionar un periodo de enfriamiento (*cooldown*) tras soltar cada dongle, y detectar con precisión cuándo un programador se "quema" (*burnout*) por no conseguir compilar a tiempo. 

# Instrucciones

## Compilación

```sh
make        # compila y genera el ejecutable ./codexion
make clean  # elimina los objetos (obj/)
make fclean # elimina objetos y el ejecutable
make re     # fclean + all
```

## Ejecución

El programa exige **exactamente 8 argumentos** (validados en [args.c](src/utils/args.c)):

```sh
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> <time_to_debug> <time_to_refactor> <number_of_compiles_required> <dongle_cooldown> <scheduler>
```

| Argumento | Descripción |
|---|---|
| `number_of_coders` | Número de programadores (hilos) a simular |
| `time_to_burnout` | Tiempo máximo (ms) sin compilar antes de "quemarse" |
| `time_to_compile` | Duración (ms) de la fase de compilación |
| `time_to_debug` | Duración (ms) de la fase de debug |
| `time_to_refactor` | Duración (ms) de la fase de refactor |
| `number_of_compiles_required` | Nº de compilaciones que debe alcanzar cada coder para que la simulación termine con éxito (`0` = sin límite) |
| `dongle_cooldown` | Tiempo (ms) que un dongle permanece bloqueado tras soltarse |
| `scheduler` | Política de la cola de espera de cada dongle: `fifo` o `edf` |

Todos los valores numéricos deben ser enteros positivos; `scheduler` debe ser literalmente `fifo` o `edf`. Cualquier argumento inválido aborta la ejecución con `Arg <nombre> is invalid.`

Ejemplo:

```sh
./codexion 5 800 200 100 100 7 100 edf
```

# Recursos
[Condiciones de Coffman](https://1984.lsi.us.es/wiki-ssoo/index.php/Condiciones_para_el_interbloqueo_y_estrategias_de_resoluci%C3%B3n)

[Uso de pthread](https://www.geeksforgeeks.org/c/thread-functions-in-c-c/)

[Curso de arquitectura: Hilos](https://www.it.uc3m.es/pbasanta/asng/course_notes/c_threads_functions_es.html)

[Programación en C: Linux C Threads](https://www.youtube.com/watch?v=tvgU3-RzAqk&list=PL19snTOMdnWv3-ceesGoZ9FhKqPrEHJjT)

# Blocking cases handled

## 1. Prevención de interbloqueos (deadlock) y condiciones de Coffman

Cada `coder` necesita **dos** dongles a la vez (izquierda y derecha), lo que reúne las cuatro condiciones de Coffman: exclusión mutua (un dongle tiene un único dueño), retención y espera (se pide un segundo recurso mientras se retiene el primero), no apropiación (nadie puede arrebatar un dongle a otro hilo) y espera circular.

- La espera circular se rompe forzando un **orden total de adquisición**: `acquire_pair()` ([simulation_cycle.c](src/simulation/simulation_cycle.c)) siempre adquiere primero el dongle de **menor id** y después el de mayor id, sin importar cuál sea "izquierda" o "derecha" para ese coder (`take_both()` en [simulation_state.c](src/simulation/simulation_state.c)). Con N hilos pidiendo recursos siempre en el mismo orden global, el ciclo A→B→A necesario para el interbloqueo no puede formarse.
- Si la segunda adquisición falla (por ejemplo, la simulación se detiene mientras el coder esperaba), el primer dongle ya obtenido se libera de inmediato (`release_dongle(low)`), evitando que un hilo se quede reteniendo un recurso que ya no va a usar.

## 2. Prevención de inanición (starvation)

Cada dongle mantiene una **cola de prioridad** (`t_heap`) con las peticiones pendientes, en lugar de dejar que los hilos compitan libremente por una única variable de condición ("sálvese quien pueda"), donde el hilo que se despierta antes podría robarle el turno a otro que lleva más tiempo esperando.

- **Política FIFO**: se sirve estrictamente en orden de llegada, usando `seq`, un contador monótono protegido por `seq_lock` (`build_request()` en [dongle.c](src/dongle/dongle.c)). Nadie puede adelantarse indefinidamente a quien llegó antes.
- **Política EDF** (*Earliest Deadline First*): se prioriza a quien tiene el *deadline* de burnout más cercano (`deadline = last_compile_start + t_burnout`), usando `seq` como desempate. Esto prioriza a quien más riesgo tiene de agotarse sin dejar de garantizar un orden determinista.
- `try_take()` ([dongle_acquire.c](src/dongle/dongle_acquire.c)) solo concede el dongle a la petición situada en la **cima del heap** (`heap_peek`): ningún hilo puede tomar el recurso "por sorpresa" solo por estar disponible, siempre se respeta el orden de la cola.

## 3. Gestión del cooldown

Tras compilar, un dongle no puede reutilizarse de inmediato:

- `release_dongle()` marca `cooldown_until = now_ms() + cooldown` bajo el mutex del propio dongle.
- `dongle_ready()` exige simultáneamente `available == 1` **y** `now_ms() >= cooldown_until`.
- `next_wake()` calcula el instante exacto en el que expira el cooldown (o un máximo de 5 ms) y lo usa como límite superior de `pthread_cond_timedwait`, de modo que los hilos en espera se despiertan justo cuando el cooldown termina, sin *busy-waiting* agresivo ni retrasos innecesarios.

## 4. Detección precisa del agotamiento (burnout)

- El hilo `monitor_routine` ([simulation_monitor.c](src/simulation/simulation_monitor.c)) recalcula, para cada coder y bajo `coder->lock`, el deadline `last_compile_start + t_burnout` y lo compara con `now_ms()` en un bucle de sondeo de alta frecuencia (`usleep(300)`), minimizando el margen entre el agotamiento real y su detección.
- `precise_sleep()` ([utils.c](src/utils/utils.c)) no duerme el intervalo completo de golpe: lo divide en tramos de 200 µs y comprueba `is_stopped()` en cada iteración, permitiendo que cualquier hilo reaccione casi instantáneamente cuando el monitor declara un burnout o el éxito de la simulación.
- La lectura de `last_compile_start` está protegida por el mismo mutex (`coder->lock`) que su escritura en `run_compile_phase()`, evitando comparar contra un valor a medio escribir.

## 5. Serialización del log

- Todas las escrituras en stdout pasan por `log_state()` ([simulation_state.c](src/simulation/simulation_state.c)), que toma `print_lock` antes de imprimir y lo libera después: los mensajes de distintos hilos (coders y monitor) nunca se intercalan.
- Dentro de esa misma sección crítica se comprueba `is_stopped()`: una vez detenida la simulación, se descarta cualquier log que no sea `STATE_BURNED`, evitando mensajes raros impresos justo después de que termine la simulación.
- Al estar la comprobación de `stop` y el `printf` dentro de la misma región protegida por `print_lock`, no hay ventana de carrera entre "decidir si logear" y "logear".

# Thread synchronization mechanisms

## Primitivas usadas

El proyecto se apoya únicamente en las primitivas estándar de pthreads — no hay ninguna implementación personalizada de eventos —, pero con un mutex por recurso en lugar de un candado global, para minimizar la contención entre hilos:

| Primitiva | Dónde | Protege |
|---|---|---|
| `pthread_mutex_t dongle->lock` | uno por cada dongle | `available`, `cooldown_until` y la cola de prioridad (`t_heap`) de ese dongle |
| `pthread_mutex_t coder->lock` | uno por cada coder | `last_compile_start`, `compiles` de ese coder |
| `pthread_mutex_t stop_lock` | global (`t_data`) | el flag `stop` de fin de simulación |
| `pthread_mutex_t seq_lock` | global (`t_data`) | el contador `seq_counter` (orden FIFO / desempate EDF) |
| `pthread_mutex_t print_lock` | global (`t_data`) | la salida por `stdout` |
| `pthread_cond_t req.cond` | uno por cada petición en cola (`t_request`) | espera/aviso de disponibilidad de un dongle concreto |

Cada petición encolada tiene su **propia** variable de condición en lugar de compartir una única `cond` por dongle. Combinado con `pthread_cond_timedwait` (nunca un `pthread_cond_wait` puro), cada hilo despertado siempre revalida por sí mismo si le toca (`try_take`) antes de continuar, en lugar de asumir que un aviso implica que el recurso es suyo.

## Cómo coordinan el acceso a los recursos compartidos

**Dongles.** `acquire_dongle()` ([dongle_acquire.c](src/dongle/dongle_acquire.c)) toma `dongle->lock`, encola la petición (`heap_push`) y entra en un bucle `while (!is_stopped())`:
1. Intenta tomar el dongle con `try_take()`: comprobar prioridad + disponibilidad y hacer `heap_pop` + `available = 0` ocurre en una única sección crítica, así que nunca dos hilos pueden ver el dongle libre y tomarlo a la vez.
2. Si no puede, calcula el próximo instante de reevaluación (`next_wake`) y llama a `pthread_cond_timedwait(&req.cond, &dongle->lock, &ts)`, que libera el mutex de forma atómica mientras espera y lo readquiere antes de volver a comprobar la condición — el patrón estándar "comprobar la condición en un bucle, con el lock tomado", que evita tanto carreras como despertares perdidos (el `timedwait` además garantiza reevaluación periódica aunque no llegue ningún `broadcast`).

`release_dongle()` toma el mismo `dongle->lock` para actualizar `available`/`cooldown_until` y para hacer `pthread_cond_broadcast` sobre todas las peticiones en cola. El *broadcast* se emite con el mutex tomado, así ningún hilo puede quedar "a medio camino" entre comprobar la condición y ponerse a esperar — se evita la carrera clásica de señalizar antes de que el receptor esté realmente escuchando.

**Log compartido.** Protegido íntegramente por `print_lock`, como se describe en la sección anterior: es la única primitiva que rodea una operación de E/S, evitando que se mezclen líneas de distintos hilos.

**Estado del monitor y comunicación thread-safe coders ↔ monitor.** El monitor no usa señales ni comparte estructuras "en crudo" con los coders: toda la comunicación pasa por memoria protegida por mutex, leída y escrita en ambos sentidos:
- *Coders → monitor*: cada coder actualiza `last_compile_start` y `compiles` bajo su propio `coder->lock` (en `run_compile_phase()`, [simulation_cycle.c](src/simulation/simulation_cycle.c)). El monitor, en `check_burnout()` y `all_done()` ([simulation_monitor.c](src/simulation/simulation_monitor.c)), lee esos mismos campos tomando el mismo `coder->lock` antes de comparar contra `now_ms()` o contra `compiles_required`. Al usar el mismo mutex en ambos lados, el monitor nunca puede leer un `last_compile_start` a medio escribir, ni un contador de compilaciones inconsistente.
- *Monitor → coders*: el monitor nunca llama directamente a un coder ni modifica su estado; se limita a escribir el flag global `stop` a través de `set_stopped()`, que toma `stop_lock`. Los coders, en cada iteración de su bucle (`coder_routine()`) y dentro de `precise_sleep()` y `acquire_dongle()`, consultan ese mismo flag mediante `is_stopped()` (mismo `stop_lock`). Así, la señal "parar" viaja de un hilo a otro exclusivamente a través de una única variable protegida por mutex, nunca por variables sin sincronizar ni por señales POSIX.
- Cuando el monitor decide detener la simulación (burnout o éxito), llama a `wake_all()` ([simulation_monitor.c](src/simulation/simulation_monitor.c)), que recorre cada dongle tomando su `dongle->lock` y hace `pthread_cond_broadcast` sobre todas las peticiones en cola. Esto es necesario porque un coder puede estar bloqueado en `pthread_cond_timedwait` esperando un dongle: sin este aviso explícito tendría que esperar hasta su próximo timeout (máx. 5 ms) para darse cuenta de que `stop` cambió — con el broadcast, la reacción es inmediata y ningún hilo se queda esperando indefinidamente al final de la simulación.
- Este esquema evita la típica condición de carrera "leer-decidir-actuar" entre monitor y coders: si el monitor comprobara `last_compile_start` sin lock mientras el coder lo está escribiendo, podría leer una mezcla de bytes antiguos y nuevos (en plataformas donde `long` no se escribe atómicamente) y declarar un burnout falso, o al revés, no detectarlo a tiempo. Al compartir siempre el mismo mutex para ese campo, la lectura del monitor y la escritura del coder quedan totalmente serializadas.

**Ejemplo concreto de condición de carrera evitada (dongles).** Si dos coders comparten un dongle (p. ej. el coder 1 usa los dongles 1 y 2, y el coder 2 usa los dongles 2 y 3) y cada uno comprobara `dongle->available` y luego lo pusiera a `0` como dos pasos sueltos sin mutex, ambos podrían leer `available == 1` a la vez y los dos empezar a compilar con el mismo dongle. Al envolver la comprobación (`dongle_ready`) y la asignación (`available = 0`) dentro de la misma sección crítica protegida por `dongle->lock` (dentro de `try_take`), esa doble concesión es imposible: solo un hilo puede ejecutar esa sección a la vez, y el otro la reintentará en su siguiente `timedwait`.
