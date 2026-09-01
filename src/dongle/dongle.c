/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fralopez <fralopez@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 10:59:11 by kurrolopez        #+#    #+#             */
/*   Updated: 2026/09/01 18:33:23 by fralopez         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Prepare a coder's request for a dongle. The deadline is calculated
** at the time of enqueuing, starting from the beginning of your last build.
*/
void	build_request(t_request *req, t_coder *coder)
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
** Is the dongle available to be granted now? It must be available and
** its cooldown must have expired.
*/
int	dongle_ready(t_dongle *dongle)
{
	if (!dongle->available)
		return (0);
	if (now_ms() < dongle->cooldown_until)
		return (0);
	return (1);
}

/*
** Returns the absolute time (timespec) for the next wake-up of
** cond_timedwait: at the latest when the cooldown expires, or 5 ms.
*/
void	next_wake(t_dongle *dongle, struct timespec *ts)
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
** Release a dongle: activate its cooldown period and wake up all
** those waiting to reassess priority.
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
