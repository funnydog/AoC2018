#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 32771

struct pos
{
	int x;
	int y;
};

static size_t hashpos(struct pos p)
{
	return p.x << 6 ^ p.y;
}

struct helem
{
	struct pos key;
	char value;
	size_t distance;

	struct helem *next;
};

struct map
{
	struct helem *table[TABLE_SIZE];
	struct pos p;
};

static struct helem *map_find(const struct map *m, struct pos p)
{
	size_t idx = hashpos(p) % TABLE_SIZE;
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
		e->distance = -1;
		size_t idx = hashpos(p) % TABLE_SIZE;
		e->next = m->table[idx];
		m->table[idx] = e;
	}
}

static char map_get(struct map *m, struct pos p)
{
	struct helem *e = map_find(m, p);
	return e ? e->value : '#';
}

static void map_print(struct map *m)
{
	int xmin, ymin, xmax, ymax;
	xmin = ymin = INT_MAX;
	xmax = ymax = INT_MIN;
	for (size_t i = 0; i < TABLE_SIZE; i++)
	{
		for (struct helem *e = m->table[i]; e; e = e->next)
		{
			if (xmin > e->key.x) xmin = e->key.x;
			if (xmax < e->key.x) xmax = e->key.x;
			if (ymin > e->key.y) ymin = e->key.y;
			if (ymax < e->key.y) ymax = e->key.y;
		}
	}

	struct pos p;
	for (int y = ymin-1; y <= ymax + 1; y++)
	{
		p.y = y;
		for (int x = xmin-1; x <= xmax + 1; x++)
		{
			p.x = x;
			putc(map_get(m, p), stdout);
		}
		putc('\n', stdout);
	}
}

static const char *map_walk(struct map *m, const char *str)
{
	struct pos orig = m->p;
	while (*str)
	{
		char value = *str++;
		switch (value)
		{
		case ')':
		case '$':
			goto out;

		case '^':
			m->p.x = m->p.y = 0;
			map_set(m, m->p, 'X');
			break;

		case '|':
			m->p = orig;
			break;

		case '(':
			str = map_walk(m, str);
			break;

		default: {
			int dx = 0, dy = 0;
			switch (value)
			{
			case 'N': dy = -1; break;
			case 'W': dx = -1; break;
			case 'S': dy = 1; break;
			case 'E': dx = 1; break;
			default: assert(0);
			}

			/* place the door */
			m->p.x += dx;
			m->p.y += dy;
			map_set(m, m->p, dx == 0 ? '-' : '|');

			/* place the room */
			m->p.x += dx;
			m->p.y += dy;
			map_set(m, m->p, '.');
			break;
		}
		}
	}
out:
	return str;
}

static void map_bfs(struct map *m, size_t *part1, size_t *part2)
{
	static const int dx[] = {0, 1, 0, -1};
	static const int dy[] = {-1, 0, 1, 0};

	struct pos p = {0};
	struct helem *e = map_find(m, p);
	assert(e);
	e->distance = 0;

	struct pos queue[256];
	size_t wi = 0, ri = 0;
	queue[wi++ & 255] = p;
	while (ri < wi)
	{
		p = queue[ri++ & 255];
		e = map_find(m, p);
		int distance = e->distance + 1;
		for (int i = 0; i < 4; i++)
		{
			struct pos np = { p.x + dx[i], p.y + dy[i] };
			e = map_find(m, np);
			if (e && e->distance == -1ULL)
			{
				e->distance = distance;
				assert(wi != ri+256);
				queue[wi++ & 255] = np;
			}
		}
	}

	*part1 = *part2 = 0;
	for (size_t i = 0; i < TABLE_SIZE; i++)
	{
		for (e = m->table[i]; e; e = e->next)
		{
			if (*part1 < e->distance)
			{
				*part1 = e->distance;
			}
			if (e->distance >= 2000 && e->value == '.')
			{
				(*part2)++;
			}
		}
	}
	*part1 /= 2;
}

static struct map *map_new(const char *str)
{
	struct map *m = calloc(1, sizeof(*m));
	if (m)
	{
		map_walk(m, str);
	}
	return m;
}

static void map_free(struct map *m)
{
	if (m)
	{
		for (size_t i = 0; i < TABLE_SIZE; i++)
		{
			struct helem *e = m->table[i];
			while (e)
			{
				struct helem *t = e;
				e = e->next;
				free(t);
			}
		}
		free(m);
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

	size_t sline = 0;
	char *line = NULL;
	getline(&line, &sline, input);
	fclose(input);
	struct map *m = map_new(line);
	free(line);
	if (!m)
	{
		fprintf(stderr, "Cannot parse the data\n");
		return 1;
	}

	(void)map_print;
	/* map_print(m); */
	size_t part1, part2;
	map_bfs(m, &part1, &part2);
	map_free(m);

	printf("Part1: %zu\nPart2: %zu\n", part1, part2);
	return 0;
}
