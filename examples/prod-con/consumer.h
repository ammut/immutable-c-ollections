//
// Created by sam on 08.05.2020.
//

#ifndef CHAMP_CONSUMER_H
#define CHAMP_CONSUMER_H

#define CONSUMER_CONTEXT_INITIALIZER ((struct consumer_context){.pool = NULL, .count = 0,})

#include "producer.h"
#include "champ.h"

struct consumer_context {
	void *pool;
	unsigned count;
};

struct code_snippet {
	unsigned id;
	unsigned version;
	char *code;
};

struct code_snippet *consume_next(struct consumer_context *ctx, struct user_story *next, struct champ *user_stories, struct champ *code_snippets);

void consumer_destroy(struct consumer_context *ctx);

#endif //CHAMP_CONSUMER_H
