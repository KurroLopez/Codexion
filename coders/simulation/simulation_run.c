/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation_run.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kurrolopez <kurrolopez@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 10:59:58 by kurrolopez        #+#    #+#             */
/*   Updated: 2026/08/26 11:00:00 by kurrolopez       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Lanza el hilo de cada coder. Detiene la simulación si alguna falla. */
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

/* Espera a que terminen todos los hilos de los coders. */
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
	printf(LOG_SUCCESS);
	return (1);
}
