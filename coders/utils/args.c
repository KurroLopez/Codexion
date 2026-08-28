/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   args.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kurrolopez <kurrolopez@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 10:58:38 by kurrolopez        #+#    #+#             */
/*   Updated: 2026/08/26 10:58:40 by kurrolopez       ###   ########.fr       */
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
