/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kurrolopez <kurrolopez@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/26 10:59:27 by kurrolopez        #+#    #+#             */
/*   Updated: 2026/08/26 10:59:28 by kurrolopez       ###   ########.fr       */
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
