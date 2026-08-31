/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation_monitor.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fralopez <fralopez@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 10:59:52 by kurrolopez        #+#    #+#             */
/*   Updated: 2026/08/31 20:47:12 by fralopez         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Despierta a todos los que esperan en cualquier dongle (para desbloquear). */
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

/* ¿Han compilado todos al menos number_of_compiles_required veces? */
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
** Comprueba burnout de un coder: se agota si no ha empezado a compilar
** dentro de time_to_burnout desde su última compilación (o el inicio).
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

void	*monitor_routine(void *arg)
{
	t_data	*data;
	int		i;

	data = (t_data *)arg;
	while (!is_stopped(data))
	{
		if (all_done(data))
		{
			set_stopped(data);
			printf(LOG_SUCCESS);
			break ;
		}
		i = 0;
		while (i < data->n)
		{
			if (check_burnout(data, i))
				printf(LOG_FAULED);
				break ;
			i++;
		}
		usleep(300);
	}
	wake_all(data);
	return (NULL);
}
