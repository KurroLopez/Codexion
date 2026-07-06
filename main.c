/* ************************************************************************** */
/*                                                                            */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	usage(void)
{
	fprintf(stderr, "Error\n");
	fprintf(stderr, "Uso: codexion number_of_coders time_to_burnout "
		"time_to_compile time_to_debug time_to_refactor "
		"number_of_compiles_required dongle_cooldown scheduler\n");
	fprintf(stderr, "  scheduler debe ser 'fifo' o 'edf'\n");
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
		fprintf(stderr, "Error: fallo de inicializacion\n");
		destroy_data(&data);
		return (1);
	}
	run_simulation(&data);
	destroy_data(&data);
	return (0);
}
