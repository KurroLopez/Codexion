/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kurrolopez <kurrolopez@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 10:59:35 by kurrolopez        #+#    #+#             */
/*   Updated: 2026/08/26 10:59:36 by kurrolopez       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_coders(t_data *data)
{
	int	i;

	data->coders = malloc(sizeof(t_coder) * data->n);
	if (!data->coders)
		return (0);
	memset(data->coders, 0, sizeof(t_coder) * data->n);
	i = 0;
	while (i < data->n)
	{
		data->coders[i].id = i + 1;
		data->coders[i].compiles = 0;
		data->coders[i].last_compile_start = data->start_ms;
		data->coders[i].data = data;
		if (pthread_mutex_init(&data->coders[i].lock, NULL) != 0)
			return (0);
		i++;
	}
	return (1);
}

static int	init_dongles(t_data *data)
{
	int	i;

	data->dongles = malloc(sizeof(t_dongle) * data->n);
	if (!data->dongles)
		return (0);
	memset(data->dongles, 0, sizeof(t_dongle) * data->n);
	i = 0;
	while (i < data->n)
	{
		data->dongles[i].id = i + 1;
		data->dongles[i].available = 1;
		data->dongles[i].cooldown_until = 0;
		data->dongles[i].data = data;
		if (pthread_mutex_init(&data->dongles[i].lock, NULL) != 0)
			return (0);
		if (!heap_init(&data->dongles[i].queue, data->n + 1))
			return (0);
		i++;
	}
	return (1);
}

int	init_data(t_data *data)
{
	data->stop = 0;
	data->seq_counter = 0;
	data->start_ms = now_ms();
	if (pthread_mutex_init(&data->print_lock, NULL) != 0)
		return (0);
	if (pthread_mutex_init(&data->stop_lock, NULL) != 0)
		return (0);
	if (pthread_mutex_init(&data->seq_lock, NULL) != 0)
		return (0);
	if (!init_coders(data))
		return (0);
	if (!init_dongles(data))
		return (0);
	return (1);
}

void	destroy_data(t_data *data)
{
	int	i;

	i = 0;
	while (data->coders && i < data->n)
	{
		pthread_mutex_destroy(&data->coders[i].lock);
		i++;
	}
	i = 0;
	while (data->dongles && i < data->n)
	{
		pthread_mutex_destroy(&data->dongles[i].lock);
		heap_free(&data->dongles[i].queue);
		i++;
	}
	free(data->coders);
	free(data->dongles);
	pthread_mutex_destroy(&data->print_lock);
	pthread_mutex_destroy(&data->stop_lock);
	pthread_mutex_destroy(&data->seq_lock);
}
