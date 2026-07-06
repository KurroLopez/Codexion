/* ************************************************************************** */
/*                                                                            */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Prepara la solicitud de un coder para un dongle. El deadline se calcula
** en el momento de encolar, a partir del inicio de su última compilación.
*/
static void	build_request(t_request *req, t_coder *coder)
{
	t_data	*data;

	data = coder->data;
	req->coder_id = coder->id;
	req->granted = 0;
	pthread_mutex_lock(&data->seq_lock);
	req->seq = data->seq_counter++;
	pthread_mutex_unlock(&data->seq_lock);
	pthread_mutex_lock(&coder->lock);
	req->deadline = coder->last_compile_start + data->t_burnout;
	pthread_mutex_unlock(&coder->lock);
	pthread_cond_init(&req->cond, NULL);
}

/*
** ¿Está el dongle libre para concederse ahora? Debe estar disponible y su
** cooldown debe haber expirado.
*/
static int	dongle_ready(t_dongle *dongle)
{
	if (!dongle->available)
		return (0);
	if (now_ms() < dongle->cooldown_until)
		return (0);
	return (1);
}

/*
** Devuelve el instante absoluto (timespec) para el próximo despertar del
** cond_timedwait: como muy tarde cuando expire el cooldown, o 5 ms.
*/
static void	next_wake(t_dongle *dongle, struct timespec *ts)
{
	long	target;
	long	nowms;

	nowms = now_ms();
	target = nowms + 5;
	if (dongle->cooldown_until > nowms && dongle->cooldown_until < target)
		target = dongle->cooldown_until;
	ts->tv_sec = target / 1000L;
	ts->tv_nsec = (target % 1000L) * 1000000L;
}

/*
** Intenta adquirir un dongle. Se encola en la cola de prioridad y espera
** hasta que es la solicitud de mayor prioridad y el dongle está listo.
** Devuelve 1 si lo consigue, 0 si la simulación se detuvo mientras esperaba.
*/
int	acquire_dongle(t_dongle *dongle, t_coder *coder)
{
	t_request		req;
	t_request		*top;
	struct timespec	ts;

	build_request(&req, coder);
	pthread_mutex_lock(&dongle->lock);
	heap_push(&dongle->queue, &req, dongle->data->sched);
	while (1)
	{
		if (is_stopped(dongle->data))
			break ;
		top = heap_peek(&dongle->queue);
		if (top == &req && dongle_ready(dongle))
		{
			heap_pop(&dongle->queue, dongle->data->sched);
			dongle->available = 0;
			pthread_mutex_unlock(&dongle->lock);
			pthread_cond_destroy(&req.cond);
			return (1);
		}
		next_wake(dongle, &ts);
		pthread_cond_timedwait(&req.cond, &dongle->lock, &ts);
	}
	heap_pop(&dongle->queue, dongle->data->sched);
	pthread_mutex_unlock(&dongle->lock);
	pthread_cond_destroy(&req.cond);
	return (0);
}

/*
** Libera un dongle: activa su periodo de enfriamiento y despierta a todos
** los que esperan para que reevalúen la prioridad.
*/
void	release_dongle(t_dongle *dongle)
{
	int	i;

	pthread_mutex_lock(&dongle->lock);
	dongle->available = 1;
	dongle->cooldown_until = now_ms() + dongle->data->cooldown;
	i = 0;
	while (i < dongle->queue.size)
	{
		pthread_cond_broadcast(&dongle->queue.items[i]->cond);
		i++;
	}
	pthread_mutex_unlock(&dongle->lock);
}
