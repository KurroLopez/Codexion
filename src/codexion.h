/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fralopez <fralopez@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 10:58:53 by kurrolopez        #+#    #+#             */
/*   Updated: 2026/08/31 20:46:28 by fralopez         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include <sys/time.h>

/* Logs messages */
# define LOG_TAKE_DONGLE	"[%ld] Coder %d has taken a dongle\n"
# define LOG_COMPILING		"[%ld] Coder %d is compiling\n"
# define LOG_DEBUGGING		"[%ld] Coder %d is debugging\n"
# define LOG_REFACTORING	"[%ld] Coder %d is refactoring\n"
# define LOG_BURNED_OUT		"[%ld] Coder %d burned out\n"
# define LOG_SUCCESS		"All coders have compiled. The team is resting.\n"
# define LOG_FAULED			"One coder is burned. Process finished.\n"
# define ERROR_ARG			"Arg <%s> is invalid.\n"

/* Person state */
typedef enum e_state
{
	STATE_TAKEN,
	STATE_COMPILING,
	STATE_DEBUGGING,
	STATE_REFACTORING,
	STATE_BURNED
}	t_state;

/* Scheduling policy */
typedef enum e_sched
{
	POL_FIFO,
	POL_EDF
}	t_sched;

typedef struct s_data	t_data;

/*
** Dongle request queued in the priority queue.
** seq: arrival order (for FIFO and tiebreaker in EDF).
** deadline: last_compile_start + time_to_burnout (for EDF).
** granted: set to 1 when the dongle is granted to this request.
** cond: the wait time based on this condition variable.
*/
typedef struct s_request
{
	int				coder_id;
	long			seq;
	long			deadline;
	int				granted;
	pthread_cond_t	cond;
}	t_request;

/*
** Binary heap-based priority queue. Stores pointers to t_request.
** The sorting criterion depends on the scheduler (FIFO or EDF).
*/
typedef struct s_heap
{
	t_request	**items;
	int			size;
	int			capacity;
}	t_heap;

/*
** A dongle on the table. Protected by its own mutex.
** available: 1 if the dongle can be taken right now.
** cooldown_until: timestamp (ms) until which the dongle is unavailable.
** queue: priority queue of people waiting for it.
*/
typedef struct s_dongle
{
	int				id;
	int				available;
	long			cooldown_until;
	pthread_mutex_t	lock;
	t_heap			queue;
	t_data			*data;
}	t_dongle;

/*
** Status of each programmer.
** last_compile_start: reference point for the burnout deadline.
** compiles: number of completed builds.
*/
typedef struct s_coder
{
	int				id;
	pthread_t		thread;
	long			last_compile_start;
	int				compiles;
	pthread_mutex_t	lock;
	t_data			*data;
}	t_coder;

/*
** Global simulation data (passed via pointer, without global data).
*/
struct s_data
{
	int				n;
	long			t_burnout;
	long			t_compile;
	long			t_debug;
	long			t_refactor;
	int				compiles_required;
	long			cooldown;
	t_sched			sched;

	long			start_ms;
	int				stop;
	long			seq_counter;

	t_coder			*coders;
	t_dongle		*dongles;

	pthread_mutex_t	print_lock;
	pthread_mutex_t	stop_lock;
	pthread_mutex_t	seq_lock;
	pthread_t		monitor;
};

/* utils.c */
long		now_ms(void);
long		elapsed_ms(t_data *data);
void		precise_sleep(t_data *data, long duration);

/* heap.c */
int			heap_init(t_heap *heap, int capacity);
void		heap_free(t_heap *heap);
int			heap_push(t_heap *heap, t_request *req, t_sched sched);
t_request	*heap_peek(t_heap *heap);
t_request	*heap_pop(t_heap *heap, t_sched sched);
void		heap_remove(t_heap *heap, t_request *req, t_sched sched);

/* heap_utils.c */
int			higher_priority(t_request *a, t_request *b, t_sched sched);
void		swap_items(t_heap *heap, int i, int j);
void		sift_up(t_heap *heap, int i, t_sched sched);
void		sift_down(t_heap *heap, int i, t_sched sched);

/* dongle.c */
void		build_request(t_request *req, t_coder *coder);
int			dongle_ready(t_dongle *dongle);
void		release_dongle(t_dongle *dongle);

/* dongle_acquire.c */
int			acquire_dongle(t_dongle *dongle, t_coder *coder);

/* args.c */
int			parse_args(int argc, char **argv, t_data *data);

/* init.c */
int			init_data(t_data *data);
void		destroy_data(t_data *data);

/* simulation_state.c */
void		log_state(t_data *data, int id, t_state state);
int			is_stopped(t_data *data);
void		set_stopped(t_data *data);
int			take_both(t_coder *coder, t_dongle *low, t_dongle *high);

/* simulation_cycle.c */
void		*coder_routine(void *arg);

/* simulation_monitor.c */
void		wake_all(t_data *data);
void		*monitor_routine(void *arg);

/* simulation_run.c */
int			run_simulation(t_data *data);

#endif
