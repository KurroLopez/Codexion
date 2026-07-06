/* ************************************************************************** */
/*                                                                            */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	heap_init(t_heap *heap, int capacity)
{
	if (capacity < 1)
		capacity = 1;
	heap->items = malloc(sizeof(t_request *) * capacity);
	if (!heap->items)
		return (0);
	heap->size = 0;
	heap->capacity = capacity;
	return (1);
}

void	heap_free(t_heap *heap)
{
	if (heap->items)
		free(heap->items);
	heap->items = NULL;
	heap->size = 0;
	heap->capacity = 0;
}

/*
** Devuelve 1 si "a" tiene mayor prioridad que "b" (debe salir antes).
** FIFO: menor número de secuencia (llegó primero).
** EDF : menor deadline; si empatan, menor secuencia.
*/
static int	higher_priority(t_request *a, t_request *b, t_sched sched)
{
	if (sched == POL_EDF)
	{
		if (a->deadline != b->deadline)
			return (a->deadline < b->deadline);
		return (a->seq < b->seq);
	}
	return (a->seq < b->seq);
}

static void	swap_items(t_heap *heap, int i, int j)
{
	t_request	*tmp;

	tmp = heap->items[i];
	heap->items[i] = heap->items[j];
	heap->items[j] = tmp;
}

static void	sift_up(t_heap *heap, int i, t_sched sched)
{
	int	parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (higher_priority(heap->items[i], heap->items[parent], sched))
		{
			swap_items(heap, i, parent);
			i = parent;
		}
		else
			break ;
	}
}

int	heap_push(t_heap *heap, t_request *req, t_sched sched)
{
	t_request	**bigger;
	int			new_cap;

	if (heap->size >= heap->capacity)
	{
		new_cap = heap->capacity * 2;
		bigger = malloc(sizeof(t_request *) * new_cap);
		if (!bigger)
			return (0);
		memcpy(bigger, heap->items, sizeof(t_request *) * heap->size);
		free(heap->items);
		heap->items = bigger;
		heap->capacity = new_cap;
	}
	heap->items[heap->size] = req;
	sift_up(heap, heap->size, sched);
	heap->size++;
	return (1);
}

t_request	*heap_peek(t_heap *heap)
{
	if (heap->size == 0)
		return (NULL);
	return (heap->items[0]);
}

static void	sift_down(t_heap *heap, int i, t_sched sched)
{
	int	best;
	int	left;
	int	right;

	while (1)
	{
		best = i;
		left = 2 * i + 1;
		right = 2 * i + 2;
		if (left < heap->size
			&& higher_priority(heap->items[left], heap->items[best], sched))
			best = left;
		if (right < heap->size
			&& higher_priority(heap->items[right], heap->items[best], sched))
			best = right;
		if (best == i)
			break ;
		swap_items(heap, i, best);
		i = best;
	}
}

t_request	*heap_pop(t_heap *heap, t_sched sched)
{
	t_request	*top;

	if (heap->size == 0)
		return (NULL);
	top = heap->items[0];
	heap->size--;
	if (heap->size > 0)
	{
		heap->items[0] = heap->items[heap->size];
		sift_down(heap, 0, sched);
	}
	return (top);
}
