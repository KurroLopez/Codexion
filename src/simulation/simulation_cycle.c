/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation_cycle.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kurrolopez <kurrolopez@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 10:57:55 by kurrolopez        #+#    #+#             */
/*   Updated: 2026/08/26 10:57:59 by kurrolopez       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Adquiere el par de dongles del coder, siempre en orden de id ascendente,
** para romper la espera circular.
*/
static int	acquire_pair(t_coder *coder, t_dongle *left, t_dongle *right)
{
	if (left->id > right->id)
		return (take_both(coder, right, left));
	return (take_both(coder, left, right));
}

/* Compila con ambos dongles tomados y los libera al terminar. */
static void	run_compile_phase(t_coder *coder, t_dongle *left, t_dongle *right)
{
	t_data	*data;

	data = coder->data;
	pthread_mutex_lock(&coder->lock);
	coder->last_compile_start = now_ms();
	pthread_mutex_unlock(&coder->lock);
	log_state(data, coder->id, STATE_COMPILING);
	precise_sleep(data, data->t_compile);
	release_dongle(left);
	release_dongle(right);
	pthread_mutex_lock(&coder->lock);
	coder->compiles++;
	pthread_mutex_unlock(&coder->lock);
}

/* Depura y refactoriza. Devuelve 0 si la simulación se detiene a mitad. */
static int	run_debug_refactor(t_data *data, t_coder *coder)
{
	if (is_stopped(data))
		return (0);
	log_state(data, coder->id, STATE_DEBUGGING);
	precise_sleep(data, data->t_debug);
	if (is_stopped(data))
		return (0);
	log_state(data, coder->id, STATE_REFACTORING);
	precise_sleep(data, data->t_refactor);
	return (!is_stopped(data));
}

/*
** Un ciclo completo: adquirir dongles, compilar, liberar, depurar y
** refactorizar. Devuelve 0 si la simulación se detuvo a mitad.
*/
static int	do_cycle(t_coder *coder)
{
	t_data		*data;
	t_dongle	*left;
	t_dongle	*right;

	data = coder->data;
	left = &data->dongles[coder->id - 1];
	right = &data->dongles[coder->id % data->n];
	if (!acquire_pair(coder, left, right))
		return (0);
	run_compile_phase(coder, left, right);
	return (run_debug_refactor(data, coder));
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	if (coder->data->n == 1)
	{
		acquire_dongle(&coder->data->dongles[0], coder);
		log_state(coder->data, coder->id, STATE_TAKEN);
		precise_sleep(coder->data, coder->data->t_burnout + 1);
		return (NULL);
	}
	while (!is_stopped(coder->data))
	{
		if (!do_cycle(coder))
			break ;
	}
	return (NULL);
}
