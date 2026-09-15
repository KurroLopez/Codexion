/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation_monitor.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fralopez <fralopez@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 10:59:52 by kurrolopez        #+#    #+#             */
/*   Updated: 2026/09/14 20:15:47 by fralopez         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Wake all coders that are waiting for a dongle */
void	wake_all(t_data *data)
{
	int	i;
	int	j;

	i = 0;
	while (i < data->n)
	{
		pthread_mutex_lock(&data->dongles[i].lock);
		j = 0;
		while (j < data->dongles[i].queue.size)
		{
			pthread_cond_broadcast(&data->dongles[i].queue.items[j]->cond);
			j++;
		}
		pthread_mutex_unlock(&data->dongles[i].lock);
		i++;
	}
}

/* Check if everybody has compiled at least number_of_compiles_required times */
static int	all_done(t_data *data)
{
	int	i;

	if (data->compiles_required == 0)
		return (0);
	i = 0;
	while (i < data->n)
	{
		pthread_mutex_lock(&data->coders[i].lock);
		if (data->coders[i].compiles < data->compiles_required)
		{
			pthread_mutex_unlock(&data->coders[i].lock);
			return (0);
		}
		pthread_mutex_unlock(&data->coders[i].lock);
		i++;
	}
	return (1);
}

/*
** Check a coder's burnout: it runs out if it hasn't started compiling 
** within time_to_burnout since its last compilation (or start).
*/
static int	check_burnout(t_data *data, int i)
{
	long	deadline;

	pthread_mutex_lock(&data->coders[i].lock);
	deadline = data->coders[i].last_compile_start + data->t_burnout;
	pthread_mutex_unlock(&data->coders[i].lock);
	if (now_ms() > deadline)
	{
		set_stopped(data);
		log_state(data, data->coders[i].id, STATE_BURNED);
		return (1);
	}
	return (0);
}

/* Scan every coder for burnout; returns 1 (and logs) once one burned out. */
static int	scan_burnout(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->n)
	{
		if (check_burnout(data, i))
		{
			return (1);
		}
		i++;
	}
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_data	*data;

	data = (t_data *)arg;
	while (!is_stopped(data))
	{
		if (all_done(data))
		{
			set_stopped(data);
			printf(LOG_SUCCESS);
			break ;
		}
		if (scan_burnout(data))
		{
			printf(LOG_FAULED);
			break ;
		}
		wake_all(data);
		usleep(300);
	}
	wake_all(data);
	return (NULL);
}
