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
#ifndef CHAMP_CHAMP_H
#define CHAMP_CHAMP_H

#include <stdint.h>
#include <stddef.h>

#ifndef CHAMP_VERBOSITY
#define CHAMP_VERBOSITY 0
#endif

#define DEBUG_NOTICE(fmt, ...) \
            do { if (CHAMP_VERBOSITY >= 5) fprintf(stderr, "DEBUG: champ: " fmt, __VA_ARGS__); } while (0)
#define DEBUG_WARN(fmt, ...) \
            do { if (CHAMP_VERBOSITY >= 4) fprintf(stderr, "DEBUG: champ: " fmt, __VA_ARGS__); } while (0)

#ifndef CHAMP_KEY_T
#define CHAMP_KEY_T void*
#endif

#ifndef CHAMP_VALUE_T
#define CHAMP_VALUE_T void*
#endif

/**
 * These are mostly for convenience
 */

#define CHAMP_HASHFN_T(name) uint32_t (*name)(const CHAMP_KEY_T)
#define CHAMP_EQUALSFN_T(name) int (*name)(const CHAMP_KEY_T left, const CHAMP_KEY_T right)
#define CHAMP_ASSOCFN_T(name) CHAMP_VALUE_T (*name)(const CHAMP_KEY_T key, const CHAMP_VALUE_T old_value, void *user_data)
#define CHAMP_VALUE_EQUALSFN_T(name) int (*name)(const CHAMP_VALUE_T left, const CHAMP_VALUE_T right)


/**
 * These macros help with defining the various callbacks. Use them like so:
 * @code{c}
 * CHAMP_MAKE_EQUALSFN(equals_int, left, right)
 * {
 *     return left == right;
 * }
 * @endcode
 */

#define CHAMP_MAKE_HASHFN(name, arg_1) uint32_t name(const CHAMP_KEY_T arg_1)
#define CHAMP_MAKE_EQUALSFN(name, arg_l, arg_r) int name(const CHAMP_KEY_T arg_l, const CHAMP_KEY_T arg_r)
#define CHAMP_MAKE_ASSOCFN(name, key_arg, value_arg, user_data_arg) CHAMP_VALUE_T name(const CHAMP_KEY_T key_arg, const CHAMP_VALUE_T value_arg, void *user_data_arg)
#define CHAMP_MAKE_VALUE_EQUALSFN(name, arg_l, arg_r) int name(const CHAMP_VALUE_T arg_l, const CHAMP_VALUE_T arg_r)

// todo: replace with something like: "typedef struct champ champ;" to hide implementation details.
struct champ {
	volatile uint32_t ref_count;
	unsigned length;
	struct node *root;

	CHAMP_HASHFN_T(hash);
	CHAMP_EQUALSFN_T(equals);
};

/**
 * Creates a new map with the given hash and equals functions. This implementation is based on the assumption that if
 * two keys are equal, their hashes must be equal as well. This is commonly known as the Java Hashcode contract.
 *
 * The reference count of a new map is zero.
 *
 * @param hash
 * @param equals
 * @return
 */
struct champ *champ_new(CHAMP_HASHFN_T(hash), CHAMP_EQUALSFN_T(equals));

/**
 * Destroys a champ. Doesn't clean up the stored key-value-pairs.
 *
 * @param old
 */
void champ_destroy(struct champ **champ);

/**
 * Atomically increases the reference count of a map.
 *
 * @param champ
 * @return
 */
struct champ *champ_acquire(const struct champ *champ);

/**
 * Atomically decreases the reference count of a map and calls champ_destroy if it caused the count to drop to zero.
 *
 * In either case then sets the reference to NULL.
 *
 * @param champ
 */
void champ_release(struct champ **champ);

/**
 * Returns the number of entries in champ.
 *
 * @param champ
 * @return the number of entries
 */
unsigned champ_length(const struct champ *champ);

/**
 * Looks up key and sets *value_receiver to the associated value. Doesn't change value_receiver if key is not set.
 *
 * @param champ
 * @param key
 * @param found is set to 0 if key is not set
 * @return
 */
CHAMP_VALUE_T champ_get(const struct champ *champ, const CHAMP_KEY_T key, int *found);

/**
 * Returns a new map derived from champ but with key set to value.
 * If replaced is not NULL, sets it to indicate if the key is present in champ.
 *
 * Reference count of the new map is zero.
 *
 * @param champ
 * @param key
 * @param value
 * @param replaced
 * @return a new champ
 */
struct champ *champ_set(const struct champ *champ, const CHAMP_KEY_T key, const CHAMP_VALUE_T value, int *replaced);

/**
 * Returns a new map derived from champ but without a mapping for key.
 *
 * Reference count of the new map is zero.
 *
 * @param champ
 * @param key
 * @param modified
 * @return
 */
struct champ *champ_del(const struct champ *champ, const CHAMP_KEY_T key, int *modified);

/**
 * Creates a new champ with the given hash and equals functions, and inserts the given keys and values.
 * Only the first 'length' elements from keys and values are inserted.
 *
 * Reference count of the new map is zero.
 *
 * @param hash
 * @param equals
 * @param keys
 * @param values
 * @param length
 * @return
 */
struct champ *champ_of(CHAMP_HASHFN_T(hash), CHAMP_EQUALSFN_T(equals), CHAMP_KEY_T *keys, CHAMP_VALUE_T *values, size_t length);

/**
 * Returns a new map derived from champ, but with key set to the return value of fn.
 * fn is passed the key, the current value for key, and user_data.
 * If key is not present in champ, NULL is passed in place of the key and current value.
 *
 * Reference count of the new map is zero.
 *
 * @param champ
 * @param key
 * @param fn
 * @param user_data
 * @return
 */
struct champ *champ_assoc(const struct champ *champ, const CHAMP_KEY_T key, CHAMP_ASSOCFN_T(fn), const void *user_data);

/**
 * Compares two maps for equality. A lot of short-circuiting is done on the assumption that unequal hashes
 * (for both keys and values) imply inequality. This is commonly known as the Java Hashcode contract: If two values
 * are equal, their hashes must be equal as well.
 *
 * @param left
 * @param right
 * @return
 */
int champ_equals(const struct champ *left, const struct champ *right, CHAMP_VALUE_EQUALSFN_T(value_equals));

/**
 * An iterator for champ. Meant to be put on the stack.
 */
struct champ_iter {
	int stack_level;
	unsigned element_cursor;
	unsigned element_arity;
	unsigned branch_cursor_stack[8];
	unsigned branch_arity_stack[8];
	const void *node_stack[8];
};

/**
 * Initializes an iterator with a champ.
 *
 * Example:
 * @code{.c}
 * struct champ_iter iter;
 * CHAMP_KEY_T key;
 * CHAMP_VAL_T val;
 *
 * champ_iter_init(&iter, champ);
 * while(champ_iter_next(&iter, &key, &val)) {
 *     // do something with key and value
 * }
 * @endcode
 *
 * @param iter
 * @param champ
 */
void champ_iter_init(struct champ_iter *iter, const struct champ *champ);

/**
 * Advances iter and points key_receiver and value_receiver to the next pair.
 *
 * @param iter
 * @param key_receiver
 * @param value_receiver
 * @return 0 if the end of the champ has been reached
 */
int champ_iter_next(struct champ_iter *iter, CHAMP_KEY_T *key_receiver, CHAMP_VALUE_T *value_receiver);

#endif //CHAMP_CHAMP_H
