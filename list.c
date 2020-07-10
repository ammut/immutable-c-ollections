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
#include <malloc.h>

#include "list.h"

struct list {
	struct list *tail;
	void *head;
	unsigned length;
	unsigned ref_count;
};

static struct list static_instance = {
	.tail = &static_instance,
	.head = NULL,
	.length = 0,
	.ref_count = 1,
};

struct list *empty_list = &static_instance;

static struct list *create(const struct list *tail, void *head)
{
	struct list *ret = malloc(sizeof *ret);
	ret->tail = (struct list *)tail;
	ret->head = head;
	ret->length = tail->length + 1;
	ret->ref_count = 0;
	return ret;
}

struct list *list_push(const struct list *list, void *element)
{
	return create(list_acquire(list), element);
}

struct list *list_pop(const struct list *list, void **receiver)
{
	*receiver = list->head;
	return list->tail;
}

void *list_peek(const struct list *list)
{
	return list->head;
}

struct list *list_reverse(const struct list *list)
{
	struct list *ret = empty_list;
	while (list != empty_list) {
		ret = list_push(ret, list->head);
		list = list->tail;
	}
	return ret;
}

struct list *list_acquire(const struct list *list)
{
	if (list == empty_list)
		return (struct list *)list;
	atomic_fetch_add((unsigned *)&list->ref_count, 1u);
	return (struct list *)list;
}

void list_release(struct list **list)
{
	struct list *current = *list;
	*list = NULL;
	tail_call:
	if (current == empty_list) {
		return;
	} else if (atomic_fetch_sub((unsigned *)&current->ref_count, 1u) == 1u) {
		struct list *tail = current->tail;
		free(current);
		current = tail;
		goto tail_call;
	}
}
