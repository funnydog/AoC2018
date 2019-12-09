#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
	/* node types */
	ROCKY = 0,
	WET = 1,
	NARROW = 2,

	/* tools */
	NONE  = 0,
	TORCH = 1,
	CLIMB = 2,
};

struct map
{
	int depth;
	size_t tx, ty;		/* coordinates of the target */
	size_t width;
	size_t height;
	int **index;
};

struct node
{
	size_t x, y;		/* coordinates        */
	int tool;		/* tool of use */
	size_t px, py;		/* parent coordinates */
	int pt;
	int type;		/* type of the node   */
	int distance;		/* distance up to now */
	int priority;
};

struct heap
{
	struct node *pool;
	size_t count;
	size_t size;
};

struct visited
{
	struct node *pool;
	size_t count;
	size_t size;
	size_t width;
};

static inline int node_cmp(const struct node *a, const struct node *b)
{
	if (a->y < b->y) return -1;
	if (a->y > b->y) return 1;
	if (a->x < b->x) return -1;
	if (a->x > b->x) return 1;
	return a->tool - b->tool;
}

static void visited_free(struct visited *v)
{
	if (v) {
		free(v->pool);
		free(v);
	}
}

static struct visited *visited_new(void)
{
	struct visited *v = malloc(sizeof(*v));
	if (v) {
		v->pool = NULL;
		v->count = v->size = 0;
	}
	return v;
}

static int visited_find(struct visited *v, size_t x, size_t y, int tool, size_t *pos)
{
	if (v->count == 0) {
		*pos = 0;
		return 0;
	}

	struct node n = {
		.x = x,
		.y = y,
		.tool = tool,
	};
	size_t low = 0;
	size_t high = v->count-1;
	while (low < high) {
		size_t mid = low + (high - low) / 2;
		if (node_cmp(v->pool + mid, &n) < 0) {
			low = mid + 1;
		} else {
			high = mid;
		}
	}
	int cmp = node_cmp(v->pool+high, &n);
	if (cmp == 0) {
		*pos = high;
		return 1;
	} else if (cmp < 0) {
		*pos = high+1;
		return 0;
	} else {
		*pos = high;
		return 0;
	}
}

static int visited_add(struct visited *v, struct node n)
{
	size_t pos;
	if (visited_find(v, n.x, n.y, n.tool, &pos))
		return -1;

	if (v->count == v->size) {
		size_t newsize = v->size ? v->size * 2 : 1024;
		struct node *newpool = realloc(v->pool, newsize * sizeof(newpool[0]));
		if (!newpool)
			return -1;

		v->size = newsize;
		v->pool = newpool;
	}
	memmove(v->pool + pos + 1, v->pool + pos, (v->count - pos) * sizeof(v->pool[0]));
	v->pool[pos] = n;
	v->count++;
	return 0;
}

static void visited_remove(struct visited *v, struct node n)
{
	size_t pos;
	if (visited_find(v, n.x, n.y, n.tool, &pos)) {
		v->count--;
		memmove(v->pool+pos, v->pool+pos+1, (v->count - pos) * sizeof(v->pool[0]));
	}
}

static struct heap *heap_new(size_t initial)
{
	struct heap *h = malloc(sizeof(*h));
	if (!h)
		return NULL;

	h->count = 0;
	h->size = initial;
	h->pool = malloc(sizeof(h->pool[0]) * h->size);
	if (!h->pool) {
		free(h);
		return NULL;
	}

	return h;
}

static void heap_free(struct heap *h)
{
	if (h) {
		free(h->pool);
		free(h);
	}
}

static inline int heap_add(struct heap *h, struct node n)
{
	if (h->count == h->size) {
		size_t newsize = h->size ? h->size * 2 : 2;
		struct node *newpool = realloc(h->pool, newsize * sizeof(newpool[0]));
		if (!newpool)
			return -1;
		h->size = newsize;
		h->pool = newpool;
	}

	h->pool[h->count] = n;
	size_t i = h->count;
	h->count++;

	while (i > 0 && h->pool[(i-1)/2].priority > h->pool[i].priority) {
		n = h->pool[i];
		h->pool[i] = h->pool[(i-1)/2];
		h->pool[(i-1)/2] = n;
		i = (i-1)/2;
	}

	return 0;
}

static struct node heap_get(struct heap *h)
{
	struct node r = h->pool[0];
	h->count--;
	h->pool[0] = h->pool[h->count];

	size_t i = 0;
	for (;;) {
		size_t min = i;
		for (size_t j = i*2+1; j <= i*2+2; j++)
			if (j < h->count && h->pool[j].priority < h->pool[min].priority)
				min = j;

		if (min == i)
			break;

		struct node t = h->pool[i];
		h->pool[i] = h->pool[min];
		h->pool[min] = t;
		i = min;
	}

	return r;
}

static void map_free(struct map *m)
{
	if (m) {
		while (m->height-- > 0)
			free(m->index[m->height]);
		free(m->index);
		free(m);
	}
}

static inline void map_compute_index(struct map *m, size_t x0, size_t y0, size_t x1, size_t y1)
{
	for (size_t y = y0; y < y1; y++) {
		for (size_t x = x0; x < x1; x++) {
			if (x == m->tx && y == m->ty) {
				m->index[y][x] = m->depth;
			} else if (x == 0 && y == 0) {
				m->index[y][x] = m->depth;
			} else if (x == 0) {
				m->index[y][x] = (y * 48271 + m->depth) % 20183;
			} else if (y == 0) {
				m->index[y][x] = (x * 16807 + m->depth) % 20183;
			} else {
				m->index[y][x] = (m->index[y][x-1] * m->index[y-1][x] + m->depth) % 20183;
			}
		}
	}
}

static struct map *map_load(FILE *input)
{
	struct map *m = malloc(sizeof(*m));
	if (!m)
		return m;

	if (fscanf(input, " depth: %d", &m->depth) != 1) {
		free(m);
		return NULL;
	}

	if (fscanf(input, " target: %zu,%zu", &m->tx, &m->ty) != 2) {
		free(m);
		return NULL;
	}
	m->width = m->tx + 1;
	m->height = m->ty + 1;

	m->index = malloc(m->height * sizeof(m->index[0]));
	if (!m->index) {
		free(m);
		return NULL;
	}

	for (size_t i = 0; i < m->height; i++) {
		m->index[i] = calloc(m->width, sizeof(m->index[0][0]));
		if (!m->index[i]) {
			while (i-- > 0)
				free(m->index[i]);
			free(m);
			return NULL;
		}
	}

	map_compute_index(m, 0, 0, m->width, m->height);

	return m;
}

static inline int map_get_risk(struct map *m, size_t x, size_t y)
{
	if (y >= m->height) {
		size_t newsize = y * 2;
		int **newi = realloc(m->index, newsize * sizeof(newi[0]));
		if (!newi)
			abort();
		m->index = newi;
 		for (size_t y = m->height; y < newsize; y++) {
			m->index[y] = malloc(m->width * sizeof(m->index[0][0]));
			if (!m->index[y])
				abort();
		}

		map_compute_index(m, 0, m->height, m->width, newsize);
		m->height = newsize;
	}

	if (x >= m->width) {
		size_t newsize = x * 2;
		for (size_t y = 0; y < m->height; y++) {
			int *newi = realloc(m->index[y], newsize * sizeof(newi[0]));
			if (!newi)
				abort();
			m->index[y] = newi;
		}
		map_compute_index(m, m->width, 0, newsize, m->height);
		m->width = newsize;
	}

	return m->index[y][x] % 3;
}

static int map_get_whole_risk(struct map *m)
{
	int risk = 0;
	for (size_t y = 0; y < m->height; y++)
		for (size_t x = 0; x < m->width; x++)
			risk += m->index[y][x] % 3;

	return risk;
}

static int tool_allowed(int type, int tool)
{
	if (type == ROCKY && tool == NONE)
		return 0;
	if (type == WET && tool == TORCH)
		return 0;
	if (type == NARROW && tool == CLIMB)
		return 0;
	return 1;
}

static int map_ufs(struct map *m, size_t sx, size_t sy)
{
	const int offs[][2] = {
		{  0,  0 },
		{  1,  0 },
		{  0,  1 },
		{  0, -1 },
		{ -1,  0 },
	};
	struct heap *frontier = heap_new(10000);
	if (!frontier)
		abort();

	struct visited *visited = visited_new();
	if (!visited)
		abort();

	struct node cur = (struct node) {
		.x = sx,
		.y = sy,
		.tool = TORCH,
		.type = map_get_risk(m, sx, sy),
		.distance = 0,
		.priority = 0,
	};
	visited_add(visited, cur);
	heap_add(frontier, cur);
	while (frontier->count > 0) {
		cur = heap_get(frontier);
		if (cur.x == m->tx && cur.y == m->ty && cur.tool == TORCH) {
			visited_free(visited);
			heap_free(frontier);
			return cur.distance;
		}

		for (int i = 0; i < 5; i++) {
			if (cur.x == 0 && offs[i][0] < 0)
				continue;
			if (cur.y == 0 && offs[i][1] < 0)
				continue;

			sx = cur.x + offs[i][0];
			sy = cur.y + offs[i][1];
			int type = map_get_risk(m, sx, sy);

			for (int tool = 0; tool < 3; tool++) {
				if (cur.x == sx && cur.y == sy && cur.tool == tool)
					continue;
				if (!tool_allowed(cur.type, tool))
					continue;
				if (!tool_allowed(type, tool))
					continue;

				int newd = cur.distance + 1 + (cur.tool != tool ? 7 : 0);

				/* manhattan distance == admissible heuristic */
				int hint =
					((sx > m->tx) ? (sx - m->tx) : (m->tx - sx))+
					((sy > m->ty) ? (sy - m->ty) : (m->ty - sy));

				size_t pos;
				int found = visited_find(visited, sx, sy, tool, &pos);
				if (!found || newd < visited->pool[pos].distance) {
					struct node n = {
						.x = sx,
						.y = sy,
						.tool = tool,
						.px = cur.x,
						.py = cur.y,
						.pt = cur.tool,
						.type = type,
						.distance = newd,
						.priority = newd + hint,
					};
					if (!found) {
						visited_add(visited, n);
					} else {
						visited->pool[pos] = n;
					}
					heap_add(frontier, n);
				}
			}
		}
	}
	visited_free(visited);
	heap_free(frontier);
	return 0;
}

static void map_print(struct map *m)
{
	for (size_t y = 0; y < m->height; y++) {
		for (size_t x = 0; x < m->width; x++) {
			if (x == 0 && y == 0)
				putc('M', stdout);
			else if (x == m->tx && y == m->ty)
				putc('T', stdout);
			else {
				switch (map_get_risk(m, x, y)) {
				case ROCKY: putc('.', stdout); break;
				case WET: putc('=', stdout); break;
				case NARROW: putc('|', stdout); break;
				}
			}
		}
		putc('\n', stdout);
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
	fclose(input);
	if (!m) {
		fprintf(stderr, "Cannot parse the map from the given file\n");
		return -1;
	}

//	map_print(m);
	printf("area risk %d\n", map_get_whole_risk(m));
//	printf("risk at (%zu,%zu) %d\n", m->width-1, m->height-1, map_get_risk(m,15,15));
//	map_print(m);
	printf("distance: %d\n", map_ufs(m, 0, 0));
//	map_free(m);
	return 0;
}
