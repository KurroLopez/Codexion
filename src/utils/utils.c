/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fralopez <fralopez@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 11:00:19 by kurrolopez        #+#    #+#             */
/*   Updated: 2026/08/31 19:17:18 by fralopez         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Current absolute timestamp in milliseconds. */
long	now_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((long)tv.tv_sec * 1000L + (long)tv.tv_usec / 1000L);
}

/* Milliseconds elapsed since the start of the simulation. */
long	elapsed_ms(t_data *data)
{
	return (now_ms() - data->start_ms);
}

/*
** Wait "duration" ms, periodically checking if the simulation has stopped,
** to react quickly to a burnout or normal end.
*/
void	precise_sleep(t_data *data, long duration)
{
	long	target;

	target = now_ms() + duration;
	while (now_ms() < target)
	{
		if (is_stopped(data))
			return ;
		usleep(200);
	}
}
