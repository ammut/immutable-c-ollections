//
// Created by sam on 08.05.2020.
//

#include <stdatomic.h>
#include <stdlib.h>
#include <assert.h>

#include "producer.h"

#define TOTAL_COUNT 131072
#define TOTAL_COUNT2 1048000
#define UPDATE_THRESHOLD_RATIO 32
#define UPDATE_PROB 0.1

#define RANDRANGE(min, max) (min + rand() / (RAND_MAX / (max - min + 1) + 1))

/*
void set_input(struct producer_context *ctx, const char *filename)
{

	FILE * fp = fopen(filename, "rb");
	fseek(fp, 0, SEEK_END);
	size_t filesize = (size_t)ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *input = malloc(filesize + 1);
	size_t bytes_read = fread(input, 1, filesize, fp);
	if (bytes_read != filesize)
		fprintf(stderr, "WARN: filesize of %s is %ld, but %ld bytes were read", filename, filesize, bytes_read);
	input[filesize] = '\0';

	ctx->input = input;
}

static char *readline(char ** const input)
{
	char *ret = NULL;
	char *current = *input;
	for (; *current != '\0'; ++current) {
		if (*current == '\n') {
			*current = '\0';
			ret = *input;
			*input = current + 1;
			break;
		}
	}
	return ret;
}
*/

struct pool {
	struct user_story user_story;
	struct pool *next;
};

static struct user_story *us_new(struct producer_context *ctx, unsigned id, unsigned version)
{
	struct pool *usp = malloc(sizeof *usp);
	usp->user_story.story = NULL;
	usp->user_story.id = id;
	usp->user_story.version = version;
	usp->next = ctx->pool;
	ctx->pool = usp;
	return &usp->user_story;
}

static int us_is_update(struct producer_context *ctx)
{
	return (ctx->count * UPDATE_THRESHOLD_RATIO) >= ctx->total && rand() <= (UPDATE_PROB * RAND_MAX);
}

static struct user_story *produce_new(struct producer_context *ctx)
{
	return us_new(ctx, ++ctx->count, 1);
}

static struct user_story *produce_update(struct producer_context *ctx, const struct champ *state)
{
	int found = 0;
	unsigned choice = RANDRANGE(1u, ctx->count);
	struct user_story *old = champ_get(state, &choice, &found);
	assert(found);
	assert(choice == old->id);
	struct user_story *updated = us_new(ctx, old->id, old->version + 1);
	return updated;
}

struct user_story *produce_next(struct producer_context *ctx, struct champ *state)
{
	struct user_story *ret = NULL;

	if (us_is_update(ctx)) {
		ret = produce_update(ctx, state);

	} else if (ctx->count < ctx->total) {
		ret = produce_new(ctx);
	}


	champ_release(&state);
	return ret;
}

void producer_destroy(struct producer_context *ctx)
{
	struct pool *pool = ctx->pool;

	while (pool != NULL) {
		struct pool *next = pool->next;
		free(pool);
		pool = next;
	}

	free(ctx->input);
}

/*
void producer_collect(struct producer_context *ctx, struct champ *state)
{
	struct pool **indirect = &ctx->pool;

	while (*indirect != NULL) {
		struct user_story *current = &(*indirect)->user_story;
		struct user_story *found = champ_get(state, &current->id, NULL);
		if (found->version != current->version) {
			struct pool *tmp = *indirect;
			*indirect = tmp->next;
			free(tmp);
		} else {
			indirect = &(*indirect)->next;
		}
	}
}
 */
