#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 131071

enum
{
	NONE = '.',
	CLAY = '#',
	WATER = '~',
	SAND = '|',
};

struct pos
{
	int x;
	int y;
};

static const struct pos UP = (struct pos){0, -1};
static const struct pos RIGHT = (struct pos){1, 0};
static const struct pos DOWN = (struct pos){0, 1};
static const struct pos LEFT = (struct pos){-1, 0};

struct pos add(struct pos a, struct pos b)
{
	return (struct pos){a.x+b.x, a.y+b.y};
}

static size_t hashfn(struct pos p)
{
	return p.y << 6 ^ p.x;
}

struct helem
{
	struct pos key;
	char value;
	struct helem *next;
};

struct map
{
	int ymin, ymax;
	struct helem *table[TABLE_SIZE];
};

static struct helem *map_find(const struct map *m, struct pos p)
{
	size_t idx = hashfn(p) % TABLE_SIZE;
	struct helem *e = m->table[idx];
	while (e && memcmp(&e->key, &p, sizeof(p)))
	{
		e = e->next;
	}
	return e;
}

static void map_set(struct map *m, struct pos p, char value)
{
	struct helem *e = map_find(m, p);
	if (e)
	{
		e->value = value;
	}
	else
	{
		e = malloc(sizeof(*e));
		assert(e);
		e->key = p;
		e->value = value;
		size_t idx = hashfn(p) % TABLE_SIZE;
		e->next = m->table[idx];
		m->table[idx] = e;
	}
}

static char map_get(const struct map *m, struct pos p)
{
	struct helem *e = map_find(m, p);
	return e ? e->value : NONE;
}

static void map_free(struct map *m)
{
	if (m)
	{
		for (size_t i = 0; i < TABLE_SIZE; i++)
		{
			struct helem *h = m->table[i];
			while (h)
			{
				struct helem *t = h;
				h = h->next;
				free(t);
			}
		}
		free(m);
	}
}

static struct map *map_load(FILE *input)
{
	struct map *m = calloc(1, sizeof(*m));
	if (!m)
	{
		return NULL;
	}
	for (;;)
	{
		int a, b, c;
		if (fscanf(input, " x=%d, y=%d..%d", &a, &b, &c) == 3)
		{
			for (int y = b; y <= c; y++)
			{
				map_set(m, (struct pos){a, y}, CLAY);
			}
		}
		else if (fscanf(input, " y=%d, x=%d..%d", &a, &b, &c) == 3)
		{
			for (int x = b; x <= c; x++)
			{
				map_set(m, (struct pos){x, a}, CLAY);
			}
		}
		else
		{
			break;
		}
	}
	m->ymin = INT_MAX;
	m->ymax = INT_MIN;
	for (size_t i = 0; i < TABLE_SIZE; i++)
	{
		for (struct helem *e = m->table[i];
		     e;
		     e = e->next)
		{
			if (m->ymin > e->key.y) m->ymin = e->key.y;
			if (m->ymax < e->key.y) m->ymax = e->key.y;
		}
	}
	return m;
}

static void map_print(struct map *m)
{
	int xmin = INT_MAX;
	int xmax = INT_MIN;
	for (size_t i = 0; i < TABLE_SIZE; i++)
	{
		for (struct helem *e = m->table[i];
		     e;
		     e = e->next)
		{
			if (xmin > e->key.x) xmin = e->key.x;
			if (xmax < e->key.x) xmax = e->key.x;
		}
	}
	for (int y = 0; y <= m->ymax; y++)
	{
		for (int x = xmin; x <= xmax; x++)
		{
			fputc(map_get(m, (struct pos){x, y}), stdout);
		}
		fputc('\n', stdout);
	}
}

static void map_count(struct map *m, int *water, int *sand)
{
	*water = *sand = 0;
	for (size_t i = 0; i < TABLE_SIZE; i++)
	{
		for (struct helem *e = m->table[i];
		     e;
		     e = e->next)
		{
			if (e->key.y < m->ymin)
			{
			}
			else if (e->value == SAND)
			{
				(*sand)++;
			}
			else if (e->value == WATER)
			{
				(*water)++;
			}
		}
	}
}

static int map_open(struct map *m, struct pos p)
{
	int v = map_get(m, p);
	return v == NONE || v == SAND;
}

static void map_fill(struct map *m, struct pos p)
{
	if (p.y > m->ymax || !map_open(m, p))
	{
		return;
	}

	if (!map_open(m, add(p, DOWN)))
	{
		struct pos left;
		for (left = p;
		     map_open(m, left) && !map_open(m, add(left, DOWN));
		     left = add(left,LEFT))
		{
			map_set(m, left, SAND);
		}

		struct pos right;
		for (right = add(p, RIGHT);
		     map_open(m, right) && !map_open(m, add(right, DOWN));
		     right = add(right, RIGHT))
		{
			map_set(m, right, SAND);
		}

		if (map_open(m, left) || map_open(m, right))
		{
			map_fill(m, left);
			map_fill(m, right);
		}
		else
		{
			left.x++;
			while (left.x != right.x)
			{
				map_set(m, left, WATER);
				left.x++;
			}
			map_fill(m, add (p, UP));
		}
	}
	else if (map_get(m, p) == NONE)
	{
		map_set(m, p, SAND);
		map_fill(m, add(p, DOWN));
	}
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

	struct map *m = map_load(input);
	fclose(input);
	(void)map_print;
	/* map_print(m); */

	map_fill(m, (struct pos){500, 0});
	/* map_print(m); */

	int water, sand;
	map_count(m, &water, &sand);
	map_free(m);

	printf("Part1: %d\n", sand + water);
	printf("Part2: %d\n", water);
	return 0;
}
