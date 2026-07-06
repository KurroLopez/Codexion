/* ************************************************************************** */
/*                                                                            */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	parse_scheduler(const char *s, t_sched *out)
{
	if (strcmp(s, "fifo") == 0)
	{
		*out = POL_FIFO;
		return (1);
	}
	if (strcmp(s, "edf") == 0)
	{
		*out = POL_EDF;
		return (1);
	}
	return (0);
}

/*
** Valida y carga los 8 argumentos obligatorios. Rechaza no enteros,
** negativos, number_of_coders < 1 y schedulers desconocidos.
*/
int	parse_args(int argc, char **argv, t_data *data)
{
	long	n;

	if (argc != 9)
		return (0);
	if (!parse_long(argv[1], &n) || n < 1)
		return (0);
	data->n = (int)n;
	if (!parse_long(argv[2], &data->t_burnout))
		return (0);
	if (!parse_long(argv[3], &data->t_compile))
		return (0);
	if (!parse_long(argv[4], &data->t_debug))
		return (0);
	if (!parse_long(argv[5], &data->t_refactor))
		return (0);
	if (!parse_long(argv[6], &n) || n < 0)
		return (0);
	data->compiles_required = (int)n;
	if (!parse_long(argv[7], &data->cooldown))
		return (0);
	if (!parse_scheduler(argv[8], &data->sched))
		return (0);
	return (1);
}

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
