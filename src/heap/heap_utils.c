/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kurrolopez <kurrolopez@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 10:59:20 by kurrolopez        #+#    #+#             */
/*   Updated: 2026/08/26 10:59:22 by kurrolopez       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Devuelve 1 si "a" tiene mayor prioridad que "b" (debe salir antes).
** FIFO: menor número de secuencia (llegó primero).
** EDF : menor deadline; si empatan, menor secuencia.
*/
int	higher_priority(t_request *a, t_request *b, t_sched sched)
{
	if (sched == POL_EDF)
	{
		if (a->deadline != b->deadline)
			return (a->deadline < b->deadline);
		return (a->seq < b->seq);
	}
	return (a->seq < b->seq);
}

void	swap_items(t_heap *heap, int i, int j)
{
	t_request	*tmp;

	tmp = heap->items[i];
	heap->items[i] = heap->items[j];
	heap->items[j] = tmp;
}

void	sift_up(t_heap *heap, int i, t_sched sched)
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

void	sift_down(t_heap *heap, int i, t_sched sched)
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
