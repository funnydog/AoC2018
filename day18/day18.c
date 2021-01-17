#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLEN 50

enum
{
	OPEN = '.',
	TREES = '|',
	LUMBER = '#',
};

struct map
{
	char area[MAXLEN][MAXLEN+1];
	char other[MAXLEN][MAXLEN+1];
	size_t width;
	size_t height;
};

static int map_eq(const struct map *a, const struct map *b)
{
	for (size_t y = 0; y < a->height; y++)
	{
		if (strcmp(a->area[y], b->area[y]))
		{
			return 0;
		}
	}
	return 1;
}

static void map_load(struct map *m, FILE *input)
{
	memset(m, 0, sizeof(*m));
	char *line = NULL;
	size_t linesize = 0;
	while (getline(&line, &linesize, input) != -1)
	{
		m->width = strlen(line);
		if (line[m->width-1] == '\n')
		{
			m->width--;
			line[m->width] = 0;
		}

		strncpy(m->area[m->height], line, MAXLEN);
		m->height++;
	}
	free(line);
}

static void map_adjacent(struct map *m, size_t x, size_t y, size_t *trees, size_t *lumber)
{
	static const int dx[] = {0, 1, 1, 1, 0, -1, -1, -1};
	static const int dy[] = {-1, -1, 0, 1, 1, 1, 0, -1};
	*trees = *lumber = 0;
	for (int i = 0; i < 8; i++)
	{
		size_t nx = x + dx[i];
		size_t ny = y + dy[i];
		if (nx < m->width && ny < m->height)
		{
			switch (m->area[ny][nx])
			{
			case TREES: (*trees)++; break;
			case LUMBER: (*lumber)++; break;
			}
		}
	}
}

static void map_next(struct map *m)
{
	for (size_t y = 0; y < m->height; y++)
	{
		for (size_t x = 0; x < m->width; x++)
		{
			size_t trees, lumber;
			map_adjacent(m, x, y, &trees, &lumber);

			char c;
			switch (m->area[y][x])
			{
			default:
			case OPEN:
				c = (trees >= 3) ? TREES : OPEN;
				break;
			case TREES:
				c = (lumber >= 3) ? LUMBER : TREES;
				break;
			case LUMBER:
				c = (lumber >= 1 && trees >= 1) ? LUMBER : OPEN;
				break;
			}
			m->other[y][x] = c;
		}
	}
	memmove(m->area, m->other, sizeof(m->area));
}

static int map_resources(struct map *m)
{
	int w = 0, l = 0;
	for (size_t y = 0; y < m->height; y++)
	{
		for (size_t x = 0; x < m->width; x++)
		{
			switch (m->area[y][x])
			{
			case '|': w++; break;
			case '#': l++; break;
			}
		}
	}
	return w * l;
}

static void map_print(struct map *m)
{
	for (size_t y = 0; y < m->height; y++)
	{
		printf("%*s\n", (int)m->width, m->area[y]);
	}
}

int main(int argc, char *argv[])
{
	(void)map_print;
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
	struct map tortoise;
	map_load(&tortoise, input);
	fclose(input);

	struct map hare = tortoise;
	size_t part1 = 0;
	size_t steps = 0;
	do
	{
		map_next(&tortoise);
		/* printf("\x1b[H"); */
		/* map_print(&tortoise); */
		map_next(&hare);
		map_next(&hare);
		steps++;
		if (steps == 10)
		{
			part1 = map_resources(&tortoise);
		}
	} while (!map_eq(&tortoise, &hare));

	size_t lam = 0;
	hare = tortoise;
	do
	{
		map_next(&hare);
		lam++;
	} while (!map_eq(&tortoise, &hare));

	size_t left = (1000000000ULL - steps ) % lam;
	while (left-->0)
	{
		map_next(&tortoise);
	}
	size_t part2 = map_resources(&tortoise);

	printf("Part1: %zu\n", part1);
	printf("Part2: %zu\n", part2);
	return 0;
}
