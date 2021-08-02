//
// Created by sam on 08.05.2020.
//

#include <stdlib.h>
#include <stdatomic.h>

#include "consumer.h"

#define LOOKUPS 40
#define LOOKUPS_THRESHOLD 32

#define RANDRANGE(min, max, seed) (min + rand_r(seed) / (RAND_MAX / (max - min + 1) + 1))

struct pool {
	struct pool *next;
	struct code_snippet code_snippet;
};

static void do_n_lookups(struct champ *map, unsigned n, unsigned seed)
{
	unsigned length = champ_length(map);
	for (unsigned i = 0; length > 0 && i < n && i < length; ++i) {
		unsigned index = RANDRANGE(1u, length, &seed);
		champ_get(map, &index, NULL);
	}
}

struct code_snippet *consume_next(struct consumer_context *ctx, struct user_story *next, struct champ *user_stories, struct champ *code_snippets)
{
	// pseudo lookups for "analyzing context", but mostly to generate some load on the champs
	do_n_lookups(user_stories, LOOKUPS, (unsigned)(uintptr_t)next);
	do_n_lookups(code_snippets, LOOKUPS, (unsigned)(uintptr_t)next);

	// further slowdown

	struct pool *csp = malloc(sizeof *csp);
	csp->code_snippet.version = next->version;
	csp->code_snippet.id = next->id;

	csp->next = ctx->pool;
	while (!atomic_compare_exchange_strong(&ctx->pool, &csp->next, csp));

	champ_release(&user_stories);
	champ_release(&code_snippets);

	return &csp->code_snippet;
}

void consumer_destroy(struct consumer_context *ctx)
{
	struct pool *pool = ctx->pool;

	while (pool != NULL) {
		struct pool *next = pool->next;
		free(pool);
		pool = next;
	}
}
