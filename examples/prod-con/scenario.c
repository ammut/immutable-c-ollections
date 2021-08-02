//
// Created by sam on 07.05.2020.
//

#include <pthread.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "queue.h"
#include "stm_rc.h"
#include "champ.h"
#include "producer.h"
#include "consumer.h"

CHAMP_MAKE_HASHFN(hash_int, id)
{
	return *(uint32_t *)id;
}

CHAMP_MAKE_EQUALSFN(equals_int, l, r)
{
	return *(unsigned *)l == *(unsigned *)r;
}








struct assoc_args {
	CHAMP_KEY_T key;
	CHAMP_ASSOCFN_T(fn);
	void *user_data;
};

atom_ref champ_assoc_va(struct champ *champ, struct assoc_args *args)
{
	return champ_assoc(champ, args->key, args->fn, args->user_data);
}

struct set_args {
	CHAMP_KEY_T key;
	CHAMP_VALUE_T value;
	int *replaced;
};

struct champ *champ_set_va(struct champ *champ, struct set_args *args)
{
	return champ_set(champ, args->key, args->value, args->replaced);
}








struct thread_ret {
	unsigned long count;
	unsigned long nsecs_total;
};

struct producer_args {
	struct atom *user_stories;
	struct atom *tasks;
	struct producer_context *producer_context;
};

union producer_thread_context {
	struct producer_args args;
	struct thread_ret ret;
};

CHAMP_MAKE_ASSOCFN(producer_insert, _, _user_story, _new_user_story)
{
	(void)_;
	const struct user_story *user_story = _user_story;
	struct user_story *new_user_story = _new_user_story;
	if (user_story) {
		new_user_story->version = user_story->version + 1;
	}
	return new_user_story;
}

struct queue *queue_close_va(struct queue *queue, void *_)
{
	(void)_;
	return queue_close(queue);
}

void *produce(union producer_thread_context *_arg)
{
	struct producer_args *arg = &_arg->args;
	struct atom *user_stories = arg->user_stories;
	struct atom *tasks = arg->tasks;
	struct producer_context *ctx = arg->producer_context;

	struct set_args set_args;

	unsigned long inserts = 0;
	unsigned long nsecs_total = 0;
	while (1) {
		struct user_story *next;

		struct timespec a, b;
		clock_gettime(CLOCK_REALTIME, &a);

		if ((next = produce_next(ctx, atom_deref(user_stories))) == NULL)
			break;

		clock_gettime(CLOCK_REALTIME, &b);
		unsigned d = b.tv_sec == a.tv_sec ?
			b.tv_nsec - a.tv_nsec :
			b.tv_nsec - (a.tv_nsec - 1000000000);
		nsecs_total += d;
		++inserts;

		int replaced = 0;
		set_args.key = &next->id;
		set_args.value = next;
		set_args.replaced = &replaced;

		struct champ *champ = atom_swap(user_stories, (atom_compute_fn)champ_set_va, &set_args);
		champ_release(&champ);

		struct queue *queue = atom_swap(tasks, (atom_compute_fn)queue_enqueue, next);
		queue_release(&queue);
	}

	struct queue *queue = atom_swap(tasks, (atom_compute_fn)queue_close_va, NULL);
	queue_release(&queue);

	_arg->ret.count = inserts;
	_arg->ret.nsecs_total = nsecs_total;

	return NULL;
}















struct consumer_args {
	struct consumer_context *consumer_context;
	struct atom *user_stories;
	struct atom *tasks;
	struct atom *code_snippets;
};

union consumer_thread_context {
	struct consumer_args args;
	struct thread_ret ret;
};

struct user_story *try_dequeue(struct atom *tasks)
{
	struct user_story *ret;
	struct queue *task_queue;

	// just do it, we might get lucky
	task_queue = atom_swap(tasks, (atom_compute_fn)queue_dequeue, &ret);

	while (ret == NULL) {
		// no luck, check if it's time to stop, or clean up and try again
		int closed = queue_is_closed(task_queue);
		queue_release(&task_queue);

		if (closed) {
			return NULL;
		}

		// try again
		task_queue = atom_swap(tasks, (atom_compute_fn)queue_dequeue, &ret);
	}

	queue_release(&task_queue);

	return ret;
}

CHAMP_MAKE_ASSOCFN(assoc_code_snippet, _, _current_snippet, _new_snippet)
{
	(void)_;
	const struct code_snippet *current_snippet = _current_snippet;
	struct code_snippet *new_snippet = _new_snippet;
	if (current_snippet != NULL && current_snippet->version > new_snippet->version)
		return (CHAMP_VALUE_T)current_snippet;
	return new_snippet;
}

CHAMP_MAKE_ASSOCFN(assoc_mapping, _, _current_snippet, _new_snippet)
{
	(void)_;
	const struct code_snippet *current_snippet = _current_snippet;
	struct code_snippet *new_snippet = _new_snippet;
	if (current_snippet != NULL && current_snippet->version > new_snippet->version)
		return (CHAMP_VALUE_T)current_snippet;
	return new_snippet;
}

void *consume(union consumer_thread_context *_arg)
{
	struct consumer_args *arg = &_arg->args;
	struct atom *user_stories = arg->user_stories;
	struct atom *tasks = arg->tasks;
	struct atom *code_snippets = arg->code_snippets;
	struct consumer_context *ctx = arg->consumer_context;
	unsigned long consumes = 0;
	unsigned long nsecs_total = 0;

	while (1) {
		struct user_story *next;
		if ((next = try_dequeue(tasks)) == NULL)
			break;

		struct timespec a, b;
		clock_gettime(CLOCK_REALTIME, &a);

		struct code_snippet *code_snippet = consume_next(ctx, next, atom_deref(user_stories), atom_deref(code_snippets));

		clock_gettime(CLOCK_REALTIME, &b);
		unsigned d = b.tv_nsec - (a.tv_nsec - (1000000000 * (b.tv_sec - a.tv_sec)));
		nsecs_total += d;
		++consumes;


		struct assoc_args assoc_args = {
			.key = &code_snippet->id,
			.fn = assoc_code_snippet,
			.user_data = next,
		};

		// insert into code_snippets - abort if newer version already available
		struct champ *cs = atom_swap(code_snippets, (atom_compute_fn)champ_assoc_va, &assoc_args);
		champ_release(&cs);
	}


	_arg->ret.count = consumes;
	_arg->ret.nsecs_total = nsecs_total;

	return NULL;
}














int main(int argc, char **argv)
{
	if (argc != 3) {
		exit:
		fprintf(stderr, "usage: scenario <threads> <user stories>");
		return 1;
	}

	int consumers_count;
	unsigned us_count;


	{
		int successful_reads = sscanf(argv[1], "%d", &consumers_count)
			+ sscanf(argv[2], "%u", &us_count);
		if (successful_reads != 2)
			goto exit;
	}

	srand(1);

	struct atom user_stories;
	atom_init(
		&user_stories,
		champ_acquire(champ_new(hash_int, equals_int)),
		(atom_ref_acquire)champ_acquire,
		(atom_ref_release)champ_release
	);

	struct atom tasks;
	atom_init(
		&tasks,
		queue_acquire(queue_new()),
		(atom_ref_acquire)queue_acquire,
		(atom_ref_release)queue_release
	);

	struct atom code_snippets;
	atom_init(
		&code_snippets,
		champ_acquire(champ_new(hash_int, equals_int)),
		(atom_ref_acquire)champ_acquire,
		(atom_ref_release)champ_release
	);

	struct producer_context p_ctx = PRODUCER_CONTEXT_INITIALIZER;
	p_ctx.total = us_count;

	struct producer_args p_args = {
		.user_stories = &user_stories,
		.tasks = &tasks,
		.producer_context = &p_ctx,
	};

	struct consumer_context c_ctx = CONSUMER_CONTEXT_INITIALIZER;

	struct consumer_args c_args = {
		.user_stories = &user_stories,
		.tasks = &tasks,
		.code_snippets = &code_snippets,
		.consumer_context = &c_ctx,
	};

	union consumer_thread_context cctx[consumers_count];
	pthread_t consumers[consumers_count];
	for (int i = 0; i < consumers_count; ++i) {
		cctx[i].args = c_args;
		pthread_create(&consumers[i], NULL, (void *(*)(void *))consume, &cctx[i]);
	}

	union producer_thread_context pctx = {.args = p_args};
	unsigned long us_produced = 0;
	unsigned long produce_next_total_time = 0;
	{
		produce(&pctx);
		struct thread_ret *ret = &pctx.ret;
		us_produced += ret->count;
		produce_next_total_time += ret->nsecs_total;
	}

	unsigned long us_consumed = 0;
	unsigned long consume_next_total_time = 0;
	for (int i = 0; i < consumers_count; ++i) {
		pthread_join(consumers[i], NULL);
		struct thread_ret *ret = &cctx[i].ret;
		us_consumed += ret->count;
		consume_next_total_time += ret->nsecs_total;
	}

	printf("user story updates pushed: %lu\n", us_produced);
	printf("user story jobs pulled: %lu\n", us_consumed);

	struct champ *us = atom_deref(&user_stories);
	printf("user stories total: %u\n", champ_length(us));
	champ_release(&us);
	struct champ *cs = atom_deref(&code_snippets);
	printf("code snippets total: %u\n", champ_length(cs));
	champ_release(&cs);

	printf("milliseconds spent in produce_next: %.3f\n", produce_next_total_time / 1000000.);
	printf("milliseconds spent in consume_next: %.3f\n", consume_next_total_time / 1000000.);

	if (us_produced)
		printf("microseconds per produce_next: %.3f\n", (produce_next_total_time / us_produced) / 1000.);
	if (us_consumed)
		printf("microseconds per consume_next: %.3f\n", (consume_next_total_time / us_consumed) / 1000.);

//	if (total_consume > 20u * us_consumed)
//		printf("total_consume: %u", consume_next_total_time);

	atom_cleanup(&user_stories);
	atom_cleanup(&code_snippets);
	atom_cleanup(&tasks);
	producer_destroy(&p_ctx);
	consumer_destroy(&c_ctx);
}
