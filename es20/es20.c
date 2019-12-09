#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
	UNKNOWN,
	ROOM,
	WALL,
	DOORV,
	DOORH,
};

struct map
{
	int xmin, xmax;
	int ymin, ymax;
	int width, height;

	int **data;
	int **dist;
};

static void minmax(struct map *m, int x, int y)
{
	if (m->xmin > x) m->xmin = x;
	if (m->xmax < x) m->xmax = x;
	if (m->ymin > y) m->ymin = y;
	if (m->ymax < y) m->ymax = y;
}

static void map_set(struct map *m, int x, int y, int v)
{
	if (x >= m->xmin && x <= m->xmax && y >= m->ymin && y <= m->ymax)
		m->data[y - m->ymin][x - m->xmin] = v;
}

static void map_set_dist(struct map *m, int x, int y, int v)
{
	if (x >= m->xmin && x <= m->xmax && y >= m->ymin && y <= m->ymax)
		m->dist[y - m->ymin][x - m->xmin] = v;
}

static int map_get(struct map *m, int x, int y)
{
	if (x >= m->xmin && x <= m->xmax && y >= m->ymin && y <= m->ymax)
		return m->data[y-m->ymin][x-m->xmin];
	return UNKNOWN;
}

static int map_get_dist(struct map *m, int x, int y)
{
	if (x >= m->xmin && x <= m->xmax && y >= m->ymin && y <= m->ymax)
		return m->dist[y-m->ymin][x-m->xmin];
	return INT_MAX;
}

static void map_print(struct map *m)
{
	for (int y = 0; y < m->height; y++) {
		for (int x = 0; x < m->width; x++) {
			if (x == -m->xmin && y == -m->ymin) {
				putc('X', stdout);
				continue;
			}
			switch (m->data[y][x]) {
			default:
			case UNKNOWN: putc('?', stdout); break;
			case ROOM:    putc('.', stdout); break;
			case DOORV:   putc('|', stdout); break;
			case DOORH:   putc('-', stdout); break;
			case WALL:    putc('#', stdout); break;
			}
		}
		putc('\n', stdout);
	}
}

static void map_print_dist(struct map *m)
{
	for (int y = 1; y < m->height; y += 2) {
		for (int x = 1; x < m->width; x += 2) {
			if (m->dist[y][x] != INT_MAX) {
				printf(" %2d", m->dist[y][x]);
			} else {
				printf(" ..");
			}
		}
		printf("\n");
	}
}

static int map_doors_max(struct map *m)
{
	int max = INT_MIN;
	for (int y = 1; y < m->height; y += 2) {
		for (int x = 1; x < m->width; x += 2) {
			int v = m->dist[y][x];
			if (v != INT_MAX && max < v)
				max = v;
		}
	}
	return max;
}

static int map_doors_gte(struct map *m, int limit)
{
	int count = 0;
	for (int y = 1; y < m->height; y += 2) {
		for (int x = 1; x < m->width; x += 2) {
			int v = m->dist[y][x];
			if (v != INT_MAX && v >= limit)
				count++;
		}
	}
	return count;
}

static void map_bfs(struct map *m, int x, int y)
{
	const int offs[][2] = {
		{  1,  0 },
		{  0,  1 },
		{ -1,  0 },
		{  0, -1 },
	};

	/* reset the distance map */
	for (int y = 0; y < m->height; y++) {
		for (int x = 0; x < m->width; x++) {
			m->dist[y][x] = INT_MAX;
		}
	}

	struct pos {
		int x, y, s;
	} queue[1000];
	int qcount = 0;
	queue[qcount++] = (struct pos){x, y, 0};
	while (qcount) {
		struct pos p = queue[0];
		qcount--;
		memmove(queue, queue+1, qcount * sizeof(queue[0]));
		map_set_dist(m, p.x, p.y, p.s);

		for (int i = 0; i < 4; i++) {
			x = p.x + offs[i][0];
			y = p.y + offs[i][1];
			int v = map_get(m, x, y);
			if (v != DOORH && v != DOORV)
				continue;

			x += offs[i][0];
			y += offs[i][1];
			if (map_get_dist(m, x, y) == INT_MAX) {
				if (qcount == 1000)
					abort();
				queue[qcount++] = (struct pos){x, y, p.s+1};
			}
		}
	}
}

static void map_free(struct map *m)
{
	if (m) {
		while (m->height-- > 0) {
			free(m->dist[m->height]);
			free(m->data[m->height]);
		}
		free(m->dist);
		free(m->data);
		free(m);
	}
}

static struct map *map_load(FILE *input)
{
	struct map *m = calloc(1, sizeof(*m));
	if (!m)
		return NULL;

	char *line = NULL;
	size_t linesize = 0;
	int x = 0, y = 0;
	struct {
		int x, y;
	} stack[1000];
	size_t scount = 0;
	while (getline(&line, &linesize, input) != -1) {
		/* TODO: simplification */

		/* map walk */
		for (char *buf = line; *buf; buf++) {
			switch(*buf) {
			case 'N': y -= 2; break;
			case 'S': y += 2; break;
			case 'W': x -= 2; break;
			case 'E': x += 2; break;
			case '(':
				if (scount == 1000)
					abort();

				stack[scount].x = x;
				stack[scount].y = y;
				scount++;
				break;

			case '|':
				if (scount == 0)
					abort();

				x = stack[scount-1].x;
				y = stack[scount-1].y;
				break;

			case ')':
				if (scount == 0)
					abort();

				scount--;
				x = stack[scount].x;
				y = stack[scount].y;
				break;

			default:
				break;
			}
			minmax(m, x, y);
		}
	}
	m->xmax++;
	m->xmin--;
	m->ymax++;
	m->ymin--;
	m->width = m->xmax - m->xmin + 1;
	m->height = m->ymax - m->ymin + 1;

	/* map allocation */
	m->data = calloc(m->height, sizeof(m->data[0]));
	m->dist = calloc(m->height, sizeof(m->dist[0]));
	if (!m->data || !m->dist) {
		map_free(m);
		return NULL;
	}
	for (int i = 0; i < m->height; i++) {
		m->data[i] = calloc(m->width, sizeof(m->data[0][0]));
		m->dist[i] = calloc(m->width, sizeof(m->dist[0][0]));
		if (!m->data[i] || !m->dist[i]) {
			map_free(m);
			return NULL;
		}
	}

	/* Map building */
	rewind(input);
	x = y = 0;
	map_set(m, x, y, ROOM);
	while (getline(&line, &linesize, input) != -1) {
		/* TODO: simplification */

		/* map walk */
		for (char *buf = line; *buf; buf++) {
			switch(*buf) {
			case 'N':
				map_set(m, x-1, y-1, WALL);
				map_set(m, x+0, y-1, DOORH);
				map_set(m, x+1, y-1, WALL);
				y -= 2;
				break;

			case 'S':
				map_set(m, x, y+1, DOORH);
				map_set(m, x-1, y+1, WALL);
				map_set(m, x+1, y+1, WALL);
				y += 2;
				break;

			case 'W':
				map_set(m, x-1, y, DOORV);
				map_set(m, x-1, y-1, WALL);
				map_set(m, x-1, y+1, WALL);
				x -= 2;
				break;

			case 'E':
				map_set(m, x+1, y, DOORV);
				map_set(m, x+1, y-1, WALL);
				map_set(m, x+1, y+1, WALL);
				x += 2;
				break;

			case '(':
				if (scount == 1000)
					abort();

				stack[scount].x = x;
				stack[scount].y = y;
				scount++;
				break;

			case '|':
				if (scount == 0)
					abort();

				x = stack[scount-1].x;
				y = stack[scount-1].y;
				break;

			case ')':
				if (scount == 0)
					abort();

				scount--;
				x = stack[scount].x;
				y = stack[scount].y;
				break;

			default:
				break;
			}
			map_set(m, x, y, ROOM);
		}
	}
	free(line);

	/* fill the unknowns */
	for (y = 0; y < m->height; y++) {
		for (x = 0; x < m->width; x++) {
			if (m->data[y][x] == UNKNOWN)
				m->data[y][x] = WALL;
		}
	}
	return m;
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
	fclose(input);

	if (m) {
//		map_print(m);
		map_bfs(m, 0, 0);
//		map_print_dist(m);
		printf("max doors: %d\n", map_doors_max(m));
		printf("at least 1000: %d\n", map_doors_gte(m, 1000));
	}

	map_free(m);
	return 0;
}
