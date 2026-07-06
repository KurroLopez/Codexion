/* ************************************************************************** */
/*                                                                            */
/*   simulation.c                                       :+:      :+:    :+:   */
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
		printf("%ld %d has taken a dongle\n", elapsed_ms(data), id);
	else if (state == STATE_COMPILING)
		printf("%ld %d is compiling\n", elapsed_ms(data), id);
	else if (state == STATE_DEBUGGING)
		printf("%ld %d is debugging\n", elapsed_ms(data), id);
	else if (state == STATE_REFACTORING)
		printf("%ld %d is refactoring\n", elapsed_ms(data), id);
	else if (state == STATE_BURNED)
		printf("%ld %d burned out\n", elapsed_ms(data), id);
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
static int	take_both(t_coder *coder, t_dongle *low, t_dongle *high)
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
	if (left->id > right->id)
	{
		if (!take_both(coder, right, left))
			return (0);
	}
	else if (!take_both(coder, left, right))
		return (0);
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

/* Despierta a todos los que esperan en cualquier dongle (para desbloquear). */
static void	wake_all(t_data *data)
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
			break ;
		}
		i = 0;
		while (i < data->n)
		{
			if (check_burnout(data, i))
				break ;
			i++;
		}
		usleep(300);
	}
	wake_all(data);
	return (NULL);
}

int	run_simulation(t_data *data)
{
	int	i;

	if (pthread_create(&data->monitor, NULL, monitor_routine, data) != 0)
		return (0);
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
	i = 0;
	while (i < data->n)
	{
		pthread_join(data->coders[i].thread, NULL);
		i++;
	}
	set_stopped(data);
	wake_all(data);
	pthread_join(data->monitor, NULL);
	return (1);
}
