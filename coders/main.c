/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fralopez <fralopez@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 10:59:42 by kurrolopez        #+#    #+#             */
/*   Updated: 2026/09/15 20:02:01 by fralopez         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	usage(void)
{
	fprintf(stderr, "Error\n");
	fprintf(stderr, "Use: codexion number_of_coders time_to_burnout "
		"time_to_compile time_to_debug time_to_refactor "
		"number_of_compiles_required dongle_cooldown scheduler\n");
	fprintf(stderr, "  scheduler should be 'fifo' or 'edf'\n");
}

int	main(int argc, char **argv)
{
	t_data	data;

	memset(&data, 0, sizeof(t_data));
	if (!parse_args(argc, argv, &data))
	{
		usage();
		return (1);
	}
	if (!init_data(&data))
	{
		fprintf(stderr, "Error: initialization failure\n");
		destroy_data(&data);
		return (1);
	}
	run_simulation(&data);
	destroy_data(&data);
	return (0);
}
