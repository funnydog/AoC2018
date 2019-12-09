#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
	TILE_NONE,
	TILE_CLAY,
	TILE_WATER,
	TILE_POUR,
};

struct map
{
	int xmin, ymin;
	int width, height;
	int **map;
};

static int map_get(struct map *m, int x, int y)
{
	if (x >= m->xmin && x < m->xmin + m->width &&
	    y >= m->ymin && y < m->ymin + m->height)
		return m->map[y - m->ymin][x - m->xmin];
	return TILE_NONE;
}

static void map_set(struct map *m, int x, int y, int value)
{
	if (x >= m->xmin && x < m->xmin + m->width &&
	    y >= m->ymin && y < m->ymin + m->height)
		m->map[y - m->ymin][x - m->xmin] = value;
}

static int map_find_left(struct map *m, int x, int y)
{
	for (; x >= m->xmin; x--) {
		if (map_get(m, x, y+1) != TILE_CLAY &&
		    map_get(m, x, y+1) != TILE_WATER)
			return 0;
		if (map_get(m, x, y) == TILE_CLAY)
			return 1;
	}
	return 0;
}

static int map_find_right(struct map *m, int x, int y)
{
	for (; x < m->xmin + m->width; x++) {
		if (map_get(m, x, y+1) != TILE_CLAY &&
		    map_get(m, x, y+1) != TILE_WATER)
			return 0;
		if (map_get(m, x, y) == TILE_CLAY)
			return 1;
	}
	return 0;
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

	/* position of the source */
	int xmin, ymin, xmax, ymax;
	xmax = ymax = INT_MIN;
	xmin = ymin = INT_MAX;

	off_t pos = ftello(input);
	int lines;
	for (lines = 0; ; lines++) {
		int a, b, c;
		int r = fscanf(input, " x=%d, y=%d..%d", &a, &b, &c);
		if (r == 3) {
			if (xmin > a) xmin = a;
			if (xmax < a) xmax = a;
			if (ymin > b) ymin = b;
			if (ymax < b) ymax = b;
			if (ymin > c) ymin = c;
			if (ymax < c) ymax = c;
			continue;
		}
		r = fscanf(input, " y=%d, x=%d..%d", &a, &b, &c);
		if (r == 3) {
			if (ymin > a) ymin = a;
			if (ymax < a) ymax = a;
			if (xmin > b) xmin = b;
			if (xmax < b) xmax = b;
			if (xmin > c) xmin = c;
			if (xmax < c) xmax = c;
			continue;
		}
		break;
	}
	printf("read %d lines\n", lines);

	/* allocate the map */
	xmin--;
	xmax++;
	m->xmin = xmin;
	m->ymin = ymin;
	m->width = xmax - xmin + 1;
	m->height = ymax - ymin + 1;
	printf("size %d,%d starting at %d,%d\n", m->width, m->height, m->xmin, m->ymin);
	m->map = malloc(m->height * sizeof(m->map[0]));
	if (m->map == NULL) {
		free(m);
		return NULL;
	}
	for (int i = 0; i < m->height; i++) {
		m->map[i] = calloc(m->width, sizeof(m->map[0][0]));
		if (m->map[i] == NULL) {
			while (i-- > 0)
				free(m->map[i]);
			free(m->map);
			free(m);
			return NULL;
		}
	}

	/* build the actual map */
	fseeko(input, pos, 0);
	while (1) {
		int x0, x1, y0, y1;
		int r = fscanf(input, " x=%d, y=%d..%d", &x0, &y0, &y1);
		if (r == 3) {
			for (int i = y0; i <= y1; i++)
				map_set(m, x0, i, TILE_CLAY);
			continue;
		}
		r = fscanf(input, " y=%d, x=%d..%d", &y0, &x0, &x1);
		if (r == 3) {
			for (int i = x0; i <= x1; i++)
				map_set(m, i, y0, TILE_CLAY);
			continue;
		}
		break;

	}
	return m;
}

static void map_print(struct map *m)
{
	for (int y = 0; y < m->height; y++) {
		for (int x = 0; x < m->width; x++) {
			int c;
			if (x + m->xmin == 500 && y + m->ymin == 0)
				c = '+';
			else if (m->map[y][x] == TILE_CLAY)
				c = '#';
			else if (m->map[y][x] == TILE_WATER)
				c = '~';
			else if (m->map[y][x] == TILE_POUR)
				c = '|';
			else
				c = '.';
//			fputc(' ', stdout);
			fputc(c, stdout);
		}
		fputc('\n', stdout);
	}
}

struct pos
{
	int x, y, p;
};

struct queue
{
	struct pos *p;
	size_t count;
	size_t size;
};

static void queue_destroy(struct queue *q)
{
	free(q->p);
}

static int queue_find(struct queue *q, struct pos p)
{
	for (size_t i = 0; i < q->count; i++) {
		if (q->p[i].x == p.x && q->p[i].y == p.y)
			return i;
	}
	return -1;
}

static void queue_add(struct queue *q, struct pos p)
{
	if (q->count == q->size) {
		size_t newsize = q->size ? q->size * 2 : 2;
		struct pos *newp = realloc(q->p, newsize * sizeof(newp[0]));
		if (!newp)
			abort();
		q->size = newsize;
		q->p = newp;
	}

	size_t pos = q->count;
	q->p[pos] = p;
	q->count++;

	while (pos > 0) {
		size_t par = (pos - 1) / 2;
		if (q->p[pos].p >= q->p[par].p)
			break;

		p = q->p[pos];
		q->p[pos] = q->p[par];
		q->p[par] = p;

		pos = par;
	}
}

static struct pos queue_pop(struct queue *q)
{
	struct pos p = q->p[0];
	q->count--;
	q->p[0] = q->p[q->count];

	/* heapify */
	size_t i = 0;
	for (;;) {
		size_t s = i;
		size_t l = i * 2 + 1;
		if (l < q->count && q->p[l].p < q->p[s].p)
			s = l;
		l++;
		if (l < q->count && q->p[l].p < q->p[s].p)
			s = l;

		if (s == i)
			break;

		struct pos t = q->p[i];
		q->p[i] = q->p[s];
		q->p[s] = t;
		i = s;
	}

	return p;
}

static void map_count(struct map *m, int *water, int *pour)
{
	*water = *pour = 0;
	for (int y = 0; y < m->height; y++) {
		for (int x = 0; x < m->width; x++) {
			if (m->map[y][x] == TILE_WATER)
				(*water)++;
			else if (m->map[y][x] == TILE_POUR)
				(*pour)++;
		}
	}
}

static int map_open(struct map *m, int x, int y)
{
	int v = map_get(m, x, y);
	return v == TILE_NONE || v == TILE_POUR;
}

static void map_fill(struct map *m)
{
	const int offs[][2] = {
		{  0, 1 },
		{  1, 0 },
		{ -1, 0 },
	};
	struct queue queue = {0};
	queue_add(&queue, (struct pos){500, m->ymin, 0});
	while (queue.count > 0) {
		struct pos p = queue_pop(&queue);

		int left = map_find_left(m, p.x, p.y);
		int right = map_find_right(m, p.x, p.y);

		if (left && right) {
			map_set(m, p.x, p.y, TILE_WATER);
			if (map_open(m, p.x-1, p.y)) {
				queue_add(&queue, (struct pos){p.x - 1, p.y, -p.y * m->width + p.x -1});
			}
			if (map_open(m, p.x+1, p.y)) {
				queue_add(&queue, (struct pos){p.x + 1, p.y, -p.y * m->width + p.x +1});
			}
			if (map_get(m, p.x, p.y-1) == TILE_POUR) {
				queue_add(&queue, (struct pos){p.x, p.y-1, -(p.y-1) * m->width + p.x});
			}
			continue;
		} else if (left) {
			for (int x = p.x; map_open(m, x, p.y); x--)
				map_set(m, x, p.y, TILE_POUR);
			queue_add(&queue, (struct pos){p.x+1, p.y, -p.y*m->width + p.x+1});
			continue;
		} else if (right) {
			for (int x = p.x; map_open(m, x, p.y); x++)
				map_set(m, x, p.y, TILE_POUR);
			queue_add(&queue, (struct pos){p.x-1, p.y, -p.y*m->width + p.x-1});
			continue;
		} else if ((p.x == 500 && p.y == m->ymin) || map_get(m, p.x, p.y-1) == TILE_POUR) {
			map_set(m, p.x, p.y, TILE_POUR);
		} else {
			if (map_get(m, p.x+1, p.y) == TILE_POUR && !map_open(m, p.x+1, p.y+1))
				map_set(m, p.x, p.y, TILE_POUR);

			if (map_get(m, p.x-1, p.y) == TILE_POUR && !map_open(m, p.x-1, p.y+1))
				map_set(m, p.x, p.y, TILE_POUR);

			if (map_get(m, p.x, p.y) != TILE_POUR)
				continue;
		}

		for (int i = 0; i < 3; i++) {
			struct pos np = {
				.x = p.x + offs[i][0],
				.y = p.y + offs[i][1],
			};
			np.p = -np.y * m->width + np.x;
			if (np.y >= m->ymin + m->height ||
			    np.x >= m->xmin + m->width ||
			    np.x < m->xmin)
				continue;

			int t = map_get(m, np.x, np.y);
			if (t == TILE_NONE) {
				if (queue_find(&queue, np) == -1)
					queue_add(&queue, np);
			}
		}
		/* map_print(m); */
		/* for (size_t i = 0; i < queue.count; i++) */
		/* 	printf(" %d,%d", queue.p[i].x, queue.p[i].y); */
		/* printf("\n"); */
	}
	queue_destroy(&queue);
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
	printf("Map size: %dx%d\n", m->width, m->height);
	map_fill(m);
	map_print(m);
	int water, pour;
	map_count(m, &water, &pour);
	printf("Watered tiles: %d\n", water+pour);
	printf("After drying: %d\n", water);
	map_free(m);

	fclose(input);
	return 0;
}
