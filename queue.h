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

#ifndef CHAMP_QUEUE_H
#define CHAMP_QUEUE_H

struct queue;

extern struct queue *empty_queue;

struct queue *queue_acquire(const struct queue *queue);
void queue_release(struct queue **queue);

struct queue *queue_new();
struct queue *queue_enqueue(const struct queue *queue, void *ref);
struct queue *queue_dequeue(const struct queue *queue, void **container);
struct queue *queue_close(const struct queue *queue);
int queue_is_closed(const struct queue *queue);

#endif //CHAMP_QUEUE_H
