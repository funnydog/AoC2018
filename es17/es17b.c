#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
	NONE,
	CLAY,
	WATER,
	SAND,
};

struct map
{
	int xmin, ymin;
	int xmax, ymax;
	int width, height;
	int **map;
};

static void map_minmax(struct map *m, int x, int y)
{
	if (m->xmin > x) m->xmin = x;
	if (m->xmax < x) m->xmax = x;
	if (m->ymin > y) m->ymin = y;
	if (m->ymax < y) m->ymax = y;
}

static int map_get(struct map *m, int x, int y)
{
	if (x >= m->xmin && x <= m->xmax && y >= m->ymin && y <= m->ymax)
		return m->map[y - m->ymin][x - m->xmin];

	return NONE;
}

static void map_set(struct map *m, int x, int y, int value)
{
	if (x >= m->xmin && x <= m->xmax && y >= m->ymin && y <= m->ymax)
		m->map[y - m->ymin][x - m->xmin] = value;
}

static void map_free(struct map *m)
{
	if (m) {
		while (m->height-- > 0)
			free(m->map[m->height]);
		free(m->map);
		free(m);
	}
}

static struct map *map_load(FILE *input)
{
	struct map *m = malloc(sizeof(*m));
	if (!m)
		return NULL;

	/* find the extents of the map */
	m->xmax = m->ymax = INT_MIN;
	m->xmin = m->ymin = INT_MAX;
	for (;;) {
		int a, b, c;
		if (fscanf(input, " x=%d, y=%d..%d", &a, &b, &c) == 3) {
			map_minmax(m, a, b);
			map_minmax(m, a, c);
		} else if (fscanf(input, " y=%d, x=%d..%d", &a, &b, &c) == 3) {
			map_minmax(m, b, a);
			map_minmax(m, c, a);
		} else {
			break;
		}
	}
	/* reserve space around the clay for the sand */
	m->xmin--;
	m->xmax++;

	/* allocate the map */
	m->width = m->xmax - m->xmin + 1;
	m->height = m->ymax - m->ymin + 1;
	m->map = malloc(m->height * sizeof(m->map[0]));
	if (!m->map) {
		free(m);
		return NULL;
	}
	for (int i = 0; i < m->height; i++) {
		m->map[i] = calloc(m->width, sizeof(m->map[0][0]));
		if (!m->map[i]) {
			while (i-- > 0)
				free(m->map[i]);
			free(m->map);
			free(m);
			return NULL;
		}
	}

	/* build the actual map */
	rewind(input);
	for (;;) {
		int a, b, c;
		if (fscanf(input, " x=%d, y=%d..%d", &a, &b, &c) == 3) {
			for (int i = b; i <= c; i++)
				map_set(m, a, i, CLAY);
		} else if (fscanf(input, " y=%d, x=%d..%d", &a, &b, &c) == 3) {
			for (int i = b; i <= c; i++)
				map_set(m, i, a, CLAY);
		} else {
			break;
		}
	}
	return m;
}

static void map_print(struct map *m)
{
	for (int y = 0; y < m->height; y++) {
		for (int x = 0; x < m->width; x++) {
			int c;
			switch (m->map[y][x]) {
			default:
			case NONE: c = '.'; break;
			case CLAY: c = '#'; break;
			case SAND: c = '|'; break;
			case WATER: c = '~'; break;
			}
			fputc(c, stdout);
		}
		fputc('\n', stdout);
	}
}

static void map_count(struct map *m, int *water, int *sand)
{
	*water = *sand = 0;
	for (int y = 0; y < m->height; y++) {
		for (int x = 0; x < m->width; x++) {
			if (m->map[y][x] == WATER)
				(*water)++;
			else if (m->map[y][x] == SAND)
				(*sand)++;
		}
	}
}

static int map_open(struct map *m, int x, int y)
{
	int v = map_get(m, x, y);
	return v == NONE || v == SAND;
}

static void map_fill(struct map *m, int x, int y)
{
	if (y >= m->ymin + m->height)
		return;
	else if (!map_open(m, x, y))
		return;
	else if (!map_open(m, x, y+1)) {
		int l;
		for (l = x; map_open(m, l, y) && !map_open(m, l, y+1); l--)
			map_set(m, l, y, SAND);

		int r;
		for (r = x+1; map_open(m, r, y) && !map_open(m, r, y+1); r++)
			map_set(m, r, y, SAND);

		if (map_open(m, l, y) || map_open(m, r, y)) {
			map_fill(m, l, y);
			map_fill(m, r, y);
		} else {
			for (int i = l+1; i < r; i++)
				map_set(m, i, y, WATER);

			map_fill(m, x, y-1);
		}
	} else if (map_get(m, x, y) == NONE) {
		map_set(m, x, y, SAND);
		map_fill(m, x, y + 1);
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return -1;
	}

	FILE *input = fopen(argv[1], "rb");
	if (!input) {
		fprintf(stderr, "Cannot open %s for reading\n", argv[1]);
		return -1;
	}

	struct map *m = map_load(input);
	/* map_print(m); */
	map_fill(m, 500, 0);
	/* map_print(m); */
	int water, sand;
	map_count(m, &water, &sand);
	printf("sand %d, water %d, sum %d\n", sand, water, sand + water);
	map_free(m);

	fclose(input);
	return 0;
}
