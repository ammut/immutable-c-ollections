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

#ifndef CHAMP_LIST_H
#define CHAMP_LIST_H

struct list;

extern struct list *empty_list;

/**
 * must be acquired
 * @param list
 * @param element
 * @return
 */
struct list *list_push(const struct list *list, void *element);

/**
 * must be acquired
 * @param list
 * @param receiver
 * @return
 */
struct list *list_pop(const struct list *list, void **receiver);

void *list_peek(const struct list *list);

unsigned list_length(const struct list *list);

/**
 * must be acquired
 * @param list
 * @return
 */
struct list *list_reverse(const struct list *list);

struct list *list_acquire(const struct list *list);

void list_release(struct list **list);

#endif //CHAMP_LIST_H
