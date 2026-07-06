/* ************************************************************************** */
/*                                                                            */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Marca temporal absoluta actual en milisegundos. */
long	now_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((long)tv.tv_sec * 1000L + (long)tv.tv_usec / 1000L);
}

/* Milisegundos transcurridos desde el inicio de la simulación. */
long	elapsed_ms(t_data *data)
{
	return (now_ms() - data->start_ms);
}

/*
** Convierte una cadena a long positivo. Solo dígitos (sin signo, sin
** espacios). Devuelve 1 en éxito, 0 si la entrada no es válida.
*/
int	parse_long(const char *s, long *out)
{
	long	value;
	int		i;

	if (!s || !s[0])
		return (0);
	value = 0;
	i = 0;
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		value = value * 10 + (s[i] - '0');
		if (value < 0)
			return (0);
		i++;
	}
	*out = value;
	return (1);
}

/*
** Espera "duration" ms comprobando periódicamente si la simulación se ha
** detenido, para reaccionar con rapidez a un burnout o al final normal.
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
