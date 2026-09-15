/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation_run.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fralopez <fralopez@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 10:59:58 by kurrolopez        #+#    #+#             */
/*   Updated: 2026/09/01 18:22:04 by fralopez         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Run a thread for each coder. Stop the simulation if one of then fail */
static void	start_coders(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->n)
	{
		if (pthread_create(&data->coders[i].thread, NULL,
				coder_routine, &data->coders[i]) != 0)
		{
			set_stopped(data);
			break ;
		}
		i++;
	}
}

/* Whait for all coders' threads are finished */
static void	join_all(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->n)
	{
		pthread_join(data->coders[i].thread, NULL);
		i++;
	}
}

int	run_simulation(t_data *data)
{
	if (pthread_create(&data->monitor, NULL, monitor_routine, data) != 0)
		return (0);
	start_coders(data);
	join_all(data);
	set_stopped(data);
	wake_all(data);
	pthread_join(data->monitor, NULL);
	return (1);
}
