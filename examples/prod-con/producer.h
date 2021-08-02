//
// Created by sam on 08.05.2020.
//

#ifndef CHAMP_PRODUCER_H
#define CHAMP_PRODUCER_H

#define PRODUCER_CONTEXT_INITIALIZER ((struct producer_context){.input = NULL, .pool = NULL, .count = 0, .total = 0})

#include <stdio.h>
#include <time.h>

#include "champ.h"

struct producer_context {
	char *input;
	void *pool;
	unsigned count;
	unsigned total;
};

struct user_story {
	const char *story;
	unsigned id;
	unsigned version;
};

struct producer_context *producer_init();

//void set_input(struct producer_context *ctx, const char *filename);

struct user_story *produce_next(struct producer_context *ctx, struct champ *state);

void producer_destroy(struct producer_context *ctx);
//void producer_collect(struct producer_context *ctx, struct champ *state);




#endif //CHAMP_PRODUCER_H
