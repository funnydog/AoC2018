#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

struct Ruleset
{
	char (*input)[5];
	size_t count;
	size_t size;
};

static void ruleset_add(struct Ruleset *r, const char *rule)
{
	if (r->count == r->size)
	{
		size_t size = r->size ? r->size * 2 : 2;
		char (*input)[5] = realloc(r->input, size * sizeof(input[0]));
		if (!input)
		{
			abort();
		}
		r->input = input;
		r->size = size;
	}
	memcpy(r->input[r->count], rule, 5);
	r->count++;
}

static int ruleset_find(const struct Ruleset *r, const char *rule)
{
	size_t low = 0;
	size_t high = r->count;
	while (low < high)
	{
		size_t mid = low + (high - low) / 2;
		if (memcmp(r->input[mid], rule, 5) < 0)
		{
			low = mid + 1;
		}
		else
		{
			high = mid;
		}
	}

	return low < r->count && memcmp(r->input[low], rule, 5) == 0;
}

struct State
{
	int pos;
	char *data;
	size_t dsize;
};

static void state_trim(struct State *s)
{
	assert(s->dsize == strlen(s->data));
	/* left */
	int len = 0;
	for (char *t = s->data + 4; *t == '.'; t++)
	{
		len++;
	}
	s->pos += len;
	s->dsize -= len;
	memmove(s->data+4, s->data+4+len, s->dsize-3);

	/* right */
	while (s->data[s->dsize-4] == '.')
	{
		s->dsize--;
		s->data[s->dsize] = 0;
	}
	assert(s->dsize == strlen(s->data));
}

static struct State *state_clone(const struct State *s)
{
	struct State *new = malloc(sizeof(*new));
	if (new)
	{
		memcpy(new, s, sizeof(*new));
		new->data = strdup(s->data);
	}
	return new;
}

static void state_free(struct State *s)
{
	if (s)
	{
		free(s->data);
		free(s);
	}
}

static void state_step(struct State *s, const struct Ruleset *r)
{
	char *new = malloc(s->dsize + 4 + 1);
	if (new)
	{
		memcpy(new, "....", 4);
		char *t = new + 4;
		for (size_t i = 0; i < s->dsize - 4; i++)
		{
			*t++ = ruleset_find(r, s->data+i) ? '#' : '.';
		}
		memcpy(t, "....", 5);
		s->pos -= 2;
		free(s->data);
		s->data = new;
		s->dsize += 4;
		state_trim(s);
	}
}

static int state_count(const struct State *s)
{
	int count = 0;
	int pos = s->pos;
	for (size_t i = 4; i < s->dsize-3; i++, pos++)
	{
		if (s->data[i] == '#')
		{
			count += pos;
		}
	}
	return count;
}

static int part1(const struct State *s, struct Ruleset *r)
{
	struct State *t = state_clone(s);
	for (size_t i = 0; i < 20; i++)
	{
		state_step(t, r);
	}

	int count = state_count(t);
	state_free(t);
	return count;
}

static int64_t part2(const struct State *s, struct Ruleset *r)
{
	struct State *tortoise = state_clone(s);
	struct State *hare = state_clone(s);
	int i = 0;
	do
	{
		state_step(tortoise, r);
		state_step(hare, r);
		state_step(hare, r);
		i++;
	}
	while (strcmp(tortoise->data, hare->data) != 0);

	int64_t cur = state_count(tortoise);
	state_step(tortoise, r);
	int64_t next = state_count(tortoise);

	state_free(hare);
	state_free(tortoise);

	return cur + (50000000000LL - i) * (next - cur);
}

static int rulecmp(const void *a, const void *b)
{
	return memcmp(a, b, 5);
}

static void parse(FILE *input, struct State *initial, struct Ruleset *r)
{
	size_t sline = 0;
	char *line = NULL;
	while (getline(&line, &sline, input) != -1)
	{
		/* chop the last \n */
		size_t len = strlen(line);
		if (line[len-1] == '\n')
		{
			line[--len] = 0;
		}

		if (line[0] == 0)
		{
			/* ignore */
		}
		else if (strncmp(line, "initial state: ", 15) == 0)
		{
			char *pos = line + 15;
			len = strlen(pos);
			initial->dsize = len + 8;
			initial->data = malloc(initial->dsize + 1);
			memcpy(initial->data, "....", 4);
			memcpy(initial->data + 4, pos, len);
			memcpy(initial->data + 4 + len, "....", 5);
			initial->pos = strchr(pos, '#') - pos;
			state_trim(initial);
		}
		else if (line[len-1] == '.')
		{
			/* ignore */
		}
		else
		{
			ruleset_add(r, line);
		}
	}
	qsort(r->input, r->count, 5, rulecmp);
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return 1;
	}

	FILE *input = fopen(argv[1], "rb");
	if (!input)
	{
		fprintf(stderr, "Cannot open %s\n", argv[1]);
		return 1;
	}

	struct State s = {0};
	struct Ruleset r = {0};
	parse(input, &s, &r);
	fclose(input);

	printf("Part1: %d\n", part1(&s, &r));
	printf("Part2: %" PRId64 "\n", part2(&s, &r));

	free(s.data);
	free(r.input);
	return 0;
}
