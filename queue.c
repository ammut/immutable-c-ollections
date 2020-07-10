/*
 * MIT License
 *
 * Copyright (c) 2020 Samuel Vogelsanger <vogelsangersamuel@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdatomic.h>
#include <stdlib.h>
#include <assert.h>

#include "queue.h"
#include "list.h"

struct queue {
	struct list *front;
	struct list *back;
	unsigned ref_count;
	int closed;
};

static struct queue *queue_swap_and_dequeue(const struct queue *q, void **container);

struct queue *queue_acquire(const struct queue *q)
{
	atomic_fetch_add(&((struct queue *)q)->ref_count, 1u);
	return (struct queue *)q;
}

void queue_release(struct queue **q)
{
	if (atomic_fetch_sub(&(*q)->ref_count, 1u) == 1) {
		struct list *front = (*q)->front;
		struct list *back = (*q)->back;
		free(*q);
		*q = NULL;
		list_release(&front);
		list_release(&back);
	}
}

struct queue *queue_new()
{
	struct queue *ret = malloc(sizeof *ret);
	ret->front = empty_list;
	ret->back = empty_list;
	ret->ref_count = 0;
	ret->closed = 0;
	return ret;
}

struct queue *queue_enqueue(const struct queue *q, void *ref)
{
	assert(!q->closed);
	if (q->closed)
		return NULL; // should provoke crashes or at least signal that something is going wrong

	struct queue *ret = malloc(sizeof *ret);
	ret->front = list_acquire(q->front);
	ret->back = list_acquire(list_push(q->back, ref));
	ret->ref_count = 0;
	ret->closed = q->closed;

	return ret;
}

struct queue *queue_dequeue(const struct queue *q, void **container)
{
	if (q->front == empty_list && q->back == empty_list) {
		*container = NULL;
		return (struct queue *)q;

	} else if (q->front == empty_list) {
		return queue_swap_and_dequeue(q, container);
	}

	struct queue *ret = malloc(sizeof *ret);
	ret->front = list_acquire(list_pop(q->front, container));
	ret->back = list_acquire(q->back);
	ret->ref_count = 0;
	ret->closed = q->closed;

	return ret;
}

struct queue *queue_close(const struct queue *q)
{
	struct queue *ret = malloc(sizeof *ret);
	ret->front = list_acquire(q->front);
	ret->back = list_acquire(q->back);
	ret->ref_count = 0;
	ret->closed = 1;
	return ret;
}

int queue_is_closed(const struct queue *q)
{
	return q->closed;
}

static struct queue *queue_swap_and_dequeue(const struct queue *q, void **container)
{
	struct list *front = list_acquire(list_reverse(q->back));
	{
		struct list *tmp = list_acquire(list_pop(front, container));
		list_release(&front);
		front = tmp;
	}

	struct queue *ret = malloc(sizeof *ret);
	ret->front = front;
	ret->back = empty_list;
	ret->ref_count = 0;
	ret->closed = q->closed;
	return ret;
}

