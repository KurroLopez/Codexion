/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_acquire.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fralopez <fralopez@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 10:59:02 by kurrolopez        #+#    #+#             */
/*   Updated: 2026/09/01 18:31:02 by fralopez         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** If the "req" request is the highest priority and the dongle is ready,
** It removes it from the queue and marks the dongle as busy.
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
** It attempts to acquire a dongle. It is queued in the priority queue and waits
** until it is the highest priority request and the dongle is ready.
** Returns 1 if successful, 0 if the simulation stopped while waiting.
*/
int	acquire_dongle(t_dongle *dongle, t_coder *coder)
{
	t_request	req;

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
		pthread_cond_wait(&req.cond, &dongle->lock);
	}
	heap_remove(&dongle->queue, &req, dongle->data->sched);
	pthread_mutex_unlock(&dongle->lock);
	pthread_cond_destroy(&req.cond);
	return (0);
}
