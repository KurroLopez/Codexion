/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   args.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fralopez <fralopez@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 10:58:38 by kurrolopez        #+#    #+#             */
/*   Updated: 2026/08/31 20:40:04 by fralopez         ###   ########.fr       */
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
** Converts a string to a positive long string. Only digits
** (unsigned, no spaces).
** Returns 1 on success, 0 if the input is invalid.
*/
static int	parse_long(const char *s, long *out)
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

static int	error_msg(const char *s)
{
	if (!s || !s[0])
		printf("Invalid number of parameters");
	else
		printf(ERROR_ARG, s);
	return (0);
}

/*
** Validate and load the 8 required arguments. Reject non-integers.
** negatives, number_of_coders < 1 and unknown schedulers.
*/
int	parse_args(int argc, char **argv, t_data *data)
{
	long	n;

	if (argc != 9)
		return (error_msg(""));
	if (!parse_long(argv[1], &n) || n < 1)
		return (error_msg("number_of_coders"));
	data->n = (int)n;
	if (!parse_long(argv[2], &data->t_burnout))
		return (error_msg("time_to_burnout"));
	if (!parse_long(argv[3], &data->t_compile))
		return (error_msg("time_to_compile"));
	if (!parse_long(argv[4], &data->t_debug))
		return (error_msg("time_to_debug"));
	if (!parse_long(argv[5], &data->t_refactor))
		return (error_msg("time_to_refactor"));
	if (!parse_long(argv[6], &n) || n < 0)
		return (error_msg("number_of_compiles_required"));
	data->compiles_required = (int)n;
	if (!parse_long(argv[7], &data->cooldown))
		return (error_msg("dongle_cooldown"));
	if (!parse_scheduler(argv[8], &data->sched))
		return (error_msg("scheduler"));
	return (1);
}
