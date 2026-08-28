/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation_state.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kurrolopez <kurrolopez@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 11:00:11 by kurrolopez        #+#    #+#             */
/*   Updated: 2026/08/26 11:00:12 by kurrolopez       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Imprime un cambio de estado de forma serializada (mutex sobre stdout). */
void	log_state(t_data *data, int id, t_state state)
{
	pthread_mutex_lock(&data->print_lock);
	if (is_stopped(data) && state != STATE_BURNED)
	{
		pthread_mutex_unlock(&data->print_lock);
		return ;
	}
	if (state == STATE_TAKEN)
		printf(LOG_TAKE_DONGLE, elapsed_ms(data), id);
	else if (state == STATE_COMPILING)
		printf(LOG_COMPILING, elapsed_ms(data), id);
	else if (state == STATE_DEBUGGING)
		printf(LOG_DEBUGGING, elapsed_ms(data), id);
	else if (state == STATE_REFACTORING)
		printf(LOG_REFACTORING, elapsed_ms(data), id);
	else if (state == STATE_BURNED)
		printf(LOG_BURNED_OUT, elapsed_ms(data), id);
	pthread_mutex_unlock(&data->print_lock);
}

int	is_stopped(t_data *data)
{
	int	value;

	pthread_mutex_lock(&data->stop_lock);
	value = data->stop;
	pthread_mutex_unlock(&data->stop_lock);
	return (value);
}

void	set_stopped(t_data *data)
{
	pthread_mutex_lock(&data->stop_lock);
	data->stop = 1;
	pthread_mutex_unlock(&data->stop_lock);
}

/*
** Adquiere ambos dongles ordenando por id menor primero (rompe la espera
** circular, condición de Coffman). Devuelve 1 si ambos se consiguen.
*/
int	take_both(t_coder *coder, t_dongle *low, t_dongle *high)
{
	if (!acquire_dongle(low, coder))
		return (0);
	log_state(coder->data, coder->id, STATE_TAKEN);
	if (!acquire_dongle(high, coder))
	{
		release_dongle(low);
		return (0);
	}
	log_state(coder->data, coder->id, STATE_TAKEN);
	return (1);
}
