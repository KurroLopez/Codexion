/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kurrolopez <kurrolopez@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 10:59:11 by kurrolopez        #+#    #+#             */
/*   Updated: 2026/08/26 10:59:13 by kurrolopez       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Prepara la solicitud de un coder para un dongle. El deadline se calcula
** en el momento de encolar, a partir del inicio de su última compilación.
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
** ¿Está el dongle libre para concederse ahora? Debe estar disponible y su
** cooldown debe haber expirado.
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
** Devuelve el instante absoluto (timespec) para el próximo despertar del
** cond_timedwait: como muy tarde cuando expire el cooldown, o 5 ms.
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
