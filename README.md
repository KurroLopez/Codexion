*This project has been created as part of the 42 curriculum by fralopez.*

# Description

`codexion` is a C program built using `pthread` in which **programmers ("coders")** require **two "dongles"** (shared USB security keys) in order to **compile**.

The goal is to simulate real concurrency using threads: avoiding deadlock and starvation, managing a cooldown period after releasing each dongle, and accurately detecting when a programmer suffers "burnout" from failing to compile in time.

# Instructions

## Compilation

```sh
make        # compiles and generates the ./codexion executable
make clean  # deletes the objects (obj/)
make fclean # removes object files and the executable
make re     # fclean + all
```

## implementation

The program requires **exactly 8 arguments** (validated in [args.c](src/utils/args.c)):

```sh
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> <time_to_debug> <time_to_refactor> <number_of_compiles_required> <dongle_cooldown> <scheduler>
```

| Argument | Description |
|---|---|
| `number_of_coders` | Number of programmers (threads) to simulate |
| `time_to_burnout` | Maximum time (ms) without compiling before "burning out" |
| `time_to_compile` | Duration (ms) of the compilation phase |
| `time_to_debug` | Duration (ms) of the debugging phase |
| `time_to_refactor` | Duration (ms) of the refactoring phase |
| `number_of_compiles_required` | Number of compilations each coder must complete for the simulation to end successfully (`0` = no limit) |
| `dongle_cooldown` | Time (ms) a dongle remains locked after being released |
| `scheduler` | Waiting queue policy for each dongle: `fifo` or `edf` |

All numeric values ​​must be positive integers; `scheduler` must be literally `fifo` or `edf`. Any invalid argument aborts execution with `Arg <name> is invalid.`

Example:

```sh
./codexion 5 800 200 100 100 7 100 edf
```

# Resources
[Condiciones de Coffman](https://1984.lsi.us.es/wiki-ssoo/index.php/Condiciones_para_el_interbloqueo_y_estrategias_de_resoluci%C3%B3n)

[Uso de pthread](https://www.geeksforgeeks.org/c/thread-functions-in-c-c/)

[Curso de arquitectura: Hilos](https://www.it.uc3m.es/pbasanta/asng/course_notes/c_threads_functions_es.html)

[Programación en C: Linux C Threads](https://www.youtube.com/watch?v=tvgU3-RzAqk&list=PL19snTOMdnWv3-ceesGoZ9FhKqPrEHJjT)

# Blocking cases handled

## 1. Deadlock prevention and Coffman conditions

Each `coder` requires **two** dongles simultaneously (left and right), satisfying the four Coffman conditions: mutual exclusion (a dongle has a single owner), hold and wait (a second resource is requested while the first is held), no preemption (no one can forcibly take a dongle from another thread), and circular wait.

- The circular wait is broken by enforcing a **total acquisition order**: `acquire_pair()` ([simulation_cycle.c](src/simulation/simulation_cycle.c)) always acquires the dongle with the **lower ID** first and then the one with the higher ID, regardless of which is "left" or "right" for that coder (`take_both()` in [simulation_state.c](src/simulation/simulation_state.c)). With N threads requesting resources always in the same global order, the A→B→A cycle required for deadlock cannot form.
- If the second acquisition fails (for example, the simulation stops while the coder is waiting), the first dongle already obtained is immediately released (`release_dongle(low)`), preventing a thread from holding onto a resource it will no longer use.
- 
## 2. Starvation prevention

Each dongle maintains a **priority queue** (`t_heap`) for pending requests, rather than allowing threads to compete freely for a single condition variable—a "free-for-all" scenario where a thread that wakes up earlier could steal the turn from another that has been waiting longer.

- **FIFO Policy**: Requests are served strictly in the order of arrival using `seq`, a monotonic counter protected by `seq_lock` (`build_request()` in [dongle.c](src/dongle/dongle.c)). No one can indefinitely jump ahead of someone who arrived earlier.
- **EDF Policy** (*Earliest Deadline First*): priority is given to the task with the nearest burnout deadline (`deadline = last_compile_start + t_burnout`), using `seq` as a tie-breaker. This prioritizes the task at greatest risk of burnout while still guaranteeing a deterministic order.
- `try_take()` ([dongle_acquire.c](src/dongle/dongle_acquire.c)) grants the dongle only to the request at the **top of the heap** (`heap_peek`); no thread can seize the resource "by surprise" simply because it is available—the queue order is always respected.

## 3. Cooldown management

After compiling, a dongle cannot be reused immediately:

- `release_dongle()` sets `cooldown_until = now_ms() + cooldown` while holding the dongle's own mutex.
- `dongle_ready()` simultaneously requires `available == 1` **and** `now_ms() >= cooldown_until`.
-`next_wake()` calculates the exact instant at which the cooldown expires (or a maximum of 5 ms) and uses it as the upper bound of `pthread_cond_timedwait`, so that waiting threads wake up just when the cooldown ends, without aggressive *busy-waiting* or unnecessary delays.

## 4. Accurate detection of burnout

- The `monitor_routine` thread ([simulation_monitor.c](src/simulation/simulation_monitor.c)) recalculates the deadline `last_compile_start + t_burnout` for each coder—holding the `coder->lock`—and compares it against `now_ms()` within a high-frequency polling loop (`usleep(300)`), thereby minimizing the gap between actual burnout and its detection.
- `precise_sleep()` ([utils.c](src/utils/utils.c)) does not sleep for the entire interval at once; instead, it breaks the time down into 200 µs segments and checks `is_stopped()` during each iteration, allowing any thread to react almost instantly when the monitor signals a burnout or the successful completion of the simulation.
- Reading `last_compile_start` is protected by the same mutex (`coder->lock`) as writing to it in `run_compile_phase()`, preventing comparison against a partially written value.

## 5. Log serialization

- All writes to stdout pass through `log_state()` ([simulation_state.c](src/simulation/simulation_state.c)), which acquires `print_lock` before printing and releases it afterwards; consequently, messages from different threads (coders and monitor) never interleave.
- Within that same critical section, `is_stopped()` is checked: once the simulation has stopped, any log other than `STATE_BURNED` is discarded, preventing odd messages from being printed immediately after the simulation ends.
- Since the check for `stop` and the `printf` are within the same region protected by `print_lock`, there is no race window between "deciding whether to log" and "logging."

# Thread synchronization mechanisms

## Used primitives

The project relies solely on standard pthreads primitives—there is no custom event implementation—but uses one mutex per resource instead of a global lock to minimize thread contention:

| Primitive | Location | Protects |
|---|---|---|
| `pthread_mutex_t dongle->lock` | one per dongle | `available`, `cooldown_until`, and the priority queue (`t_heap`) for that dongle |
| `pthread_mutex_t coder->lock` | one per coder | `last_compile_start` and `compiles` for that coder |
| `pthread_mutex_t stop_lock` | global (`t_data`) | the simulation termination flag `stop` |
| `pthread_mutex_t seq_lock` | global (`t_data`) | the `seq_counter` (FIFO order / EDF tie-breaking) |
| `pthread_mutex_t print_lock` | global (`t_data`) | `stdout` output |
| `pthread_cond_t req.cond` | one per queued request (`t_request`) | waiting/notification for the availability of a specific dongle |

Each queued request has its **own** condition variable rather than sharing a single `cond` per dongle. Combined with `pthread_cond_timedwait` (never a plain `pthread_cond_wait`), each awakened thread always independently re-verifies whether it is its turn (`try_take`) before proceeding, rather than assuming that a notification implies the resource is available to it.

## How they coordinate access to shared resources

**Dongles.** `acquire_dongle()` ([dongle_acquire.c](src/dongle/dongle_acquire.c)) acquires `dongle->lock`, enqueues the request (`heap_push`), and enters a `while (!is_stopped())` loop:
1. Try to acquire the dongle using `try_take()`: checking priority and availability, followed by `heap_pop` and setting `available = 0`, all occurs within a single critical section, so two threads can never see the dongle as free and acquire it simultaneously.
2. If it cannot, it calculates the next re-evaluation time (`next_wake`) and calls `pthread_cond_timedwait(&req.cond, &dongle->lock, &ts)`, which atomically releases the mutex while waiting and re-acquires it before re-checking the condition—the standard "check condition in a loop while holding the lock" pattern, which prevents both race conditions and missed wake-ups (the `timedwait` also guarantees periodic re-evaluation even if no `broadcast` arrives).

`release_dongle()` acquires the same `dongle->lock` to update `available`/`cooldown_until` and to call `pthread_cond_broadcast` for all queued requests. The broadcast is issued while the mutex is held, so no thread can get stuck "halfway" between checking the condition and waiting—thereby avoiding the classic race condition of signaling before the receiver is actually listening.

**Shared log.** Fully protected by `print_lock`, as described in the previous section: it is the only primitive surrounding an I/O operation, preventing lines from different threads from becoming intermingled.

**Monitor state and thread-safe communication between coders and the monitor.** The monitor does not use signals or share "raw" structures with the coders; all communication takes place via mutex-protected memory, read and written in both directions:
- *Coders → monitor*: each coder updates `last_compile_start` and `compiles` while holding its own `coder->lock` (in `run_compile_phase()`, [simulation_cycle.c](src/simulation/simulation_cycle.c)). The monitor, in `check_burnout()` and `all_done()` ([simulation_monitor.c](src/simulation/simulation_monitor.c)), reads those same fields while holding the same `coder->lock` before comparing them against `now_ms()` or `compiles_required`. By using the same mutex on both sides, the monitor never reads a partially written `last_compile_start` or an inconsistent compilation counter.
- *Monitor → coders*: the monitor never directly calls a coder or modifies its state; instead, it simply writes to the global `stop` flag via `set_stopped()`, which acquires the `stop_lock`. During each iteration of their loop (`coder_routine()`)—and within `precise_sleep()` and `acquire_dongle()`—the coders check this same flag using `is_stopped()` (utilizing the same `stop_lock`). Thus, the "stop" signal travels from one thread to another exclusively via a single mutex-protected variable, never through unsynchronized variables or POSIX signals.
- When the monitor decides to stop the simulation (due to burnout or success), it calls `wake_all()` ([simulation_monitor.c](src/simulation/simulation_monitor.c)), which iterates through each dongle, acquires its `dongle->lock`, and issues a `pthread_cond_broadcast` for all queued requests. This is necessary because a coder thread might be blocked in `pthread_cond_timedwait` while waiting for a dongle; without this explicit notification, it would have to wait until its next timeout (max. 5 ms) to realize that the `stop` flag had changed. With the broadcast, the reaction is immediate, ensuring no thread remains waiting indefinitely at the end of the simulation.
- This scheme avoids the typical "read-decide-act" race condition between the monitor and the coders: if the monitor were to check `last_compile_start` without a lock while a coder was writing to it, it could read a mix of old and new bytes (on platforms where `long` is not written atomically) and report a false burnout—or, conversely, fail to detect one in time. By always sharing the same mutex for that field, the monitor's read and the coder's write operations are fully serialized.
  
**A concrete example of a race condition being avoided (dongles).** If two coders share a dongle (e.g., Coder 1 uses dongles 1 and 2, and Coder 2 uses dongles 2 and 3) and each were to check `dongle->available` and then set it to `0` as two separate steps without a mutex, both could read `available == 1` simultaneously and start compiling using the same dongle. By wrapping the check (`dongle_ready`) and the assignment (`available = 0`) within the same critical section protected by `dongle->lock` (inside `try_take`), such double allocation becomes impossible: only one thread can execute that section at a time, and the other will retry during its next `timedwait`.
