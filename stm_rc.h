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

#ifndef CHAMP_STM_RC_H
#define CHAMP_STM_RC_H

typedef void *atom_ref;
typedef atom_ref(*atom_ref_acquire)(atom_ref);
typedef void (*atom_ref_release)(atom_ref *);
typedef atom_ref (*atom_compute_fn)(atom_ref current, void *compute_arg);

struct atom {
	atom_ref volatile ref;
	atom_ref_acquire acquire;
	atom_ref_release release;
	pthread_rwlock_t lock;
};

/**
 * Initializes an atom with a reference and takes care of the lock.
 * Does **NOT** call acquire.
 * Intended to be used in either of the following ways:
 *
 * @code
 * // variant a
 * struct atom *atom = malloc(sizeof *atom);
 * *atom = atom_init(NULL, inc_rc, dec_rc);
 *
 * // variant b
 * stuct atom = atom_init(NULL, inc_rc, dec_rc);
 * @endcode
 *
 *
 * @param ref
 * @param acquire
 * @param release
 * @return
 */
void atom_init(struct atom *atom, atom_ref ref, atom_ref_acquire acquire, atom_ref_release release);

/**
 * Decrements the reference count of the wrapped reference (by applying the release function to it). Does not free
 * the atom itself. Does also not set the reference to NULL - that should be done by the release function.
 *
 * @param atom
 */
void atom_cleanup(struct atom *atom);

/**
 * Increments the reference count of the wrapped reference (by applying the acquire function to it) and returns it.
 *
 * @param atom
 * @return
 */
void *atom_deref(struct atom *atom);

/**
 * Tries to update the wrapped reference with a replacement. Will call compute in a loop until succeeding, so compute
 * should be free of side effects. On success, increments the refcount of the new wrapped reference twice: Once for
 * the atom itself, once for the caller. If the return value is ignored at the call site, it should be decremented.
 * This also means that the return value of compute should have a reference count of zero.
 *
 * @param atom
 * @param compute
 * @param ...
 * @return
 */
void *atom_swap(struct atom *atom, atom_compute_fn compute, void *compute_arg);

#endif //CHAMP_STM_RC_H
