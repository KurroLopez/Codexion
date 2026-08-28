/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_acquire.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kurrolopez <kurrolopez@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 10:59:02 by kurrolopez        #+#    #+#             */
/*   Updated: 2026/08/26 10:59:04 by kurrolopez       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Si la solicitud "req" es la de mayor prioridad y el dongle está listo,
** la retira de la cola y marca el dongle como ocupado.
*/
static int	try_take(t_dongle *dongle, t_request *req)
{
	t_request	*top;

	top = heap_peek(&dongle->queue);
	if (top == req && dongle_ready(dongle))
	{
		heap_pop(&dongle->queue, dongle->data->sched);
		dongle->available = 0;
		return (1);
	}
	return (0);
}

/*
** Intenta adquirir un dongle. Se encola en la cola de prioridad y espera
** hasta que es la solicitud de mayor prioridad y el dongle está listo.
** Devuelve 1 si lo consigue, 0 si la simulación se detuvo mientras esperaba.
*/
int	acquire_dongle(t_dongle *dongle, t_coder *coder)
{
	t_request		req;
	struct timespec	ts;

	build_request(&req, coder);
	pthread_mutex_lock(&dongle->lock);
	heap_push(&dongle->queue, &req, dongle->data->sched);
	while (!is_stopped(dongle->data))
	{
		if (try_take(dongle, &req))
		{
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
