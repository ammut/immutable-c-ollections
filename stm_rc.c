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

#include <stdlib.h>
#include <pthread.h>

#include "stm_rc.h"

void atom_init(struct atom *atom, atom_ref ref, atom_ref_acquire acquire, atom_ref_release release)
{
	atom->ref = ref;
	atom->acquire = acquire;
	atom->release = release;
	pthread_rwlock_init(&atom->lock, NULL);
}

void atom_cleanup(struct atom *atom)
{
	pthread_rwlock_rdlock(&atom->lock);
	atom->release((atom_ref *)&atom->ref);
	pthread_rwlock_unlock(&atom->lock);
}

void *atom_deref(struct atom *atom)
{
	pthread_rwlock_rdlock(&atom->lock);
	atom_ref ret = atom->acquire(atom->ref);
	pthread_rwlock_unlock(&atom->lock);
	return ret;
}

void *atom_swap(struct atom *atom, atom_compute_fn compute, void *compute_arg)
{
	atom_ref ret;

	int done = 0;
	while (!done) {
		atom_ref current;
		atom_ref aspirant;

		current = atom_deref(atom);
		aspirant = atom->acquire(compute(current, compute_arg));

		pthread_rwlock_wrlock(&atom->lock);
		if (current == atom->ref) {
			ret = aspirant;
			atom_ref tmp = atom->ref;
			atom->ref = atom->acquire(ret);

			done = 1;
			aspirant = tmp;
		}
		pthread_rwlock_unlock(&atom->lock);

		atom->release(&current);
		atom->release(&aspirant);
	}

	return ret;
}
