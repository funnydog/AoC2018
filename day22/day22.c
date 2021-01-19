#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* tools */
#define NONE	  0
#define TORCH	  1
#define GEAR	  2

/* hash size */
#define HASH_SIZE 153281

struct state
{
	int x, y, tool;
};

struct map
{
	int depth;
	struct state target;
	size_t width;
	size_t height;
	int **index;
};

static inline size_t hashfn(const struct state *s)
{
	return (s->x << 8) ^ (s->y<<2) ^ s->tool;
}

static inline int manhattan(const struct state *a, const struct state *b)
{
	int dx = a->x - b->x;
	int dy = a->y - b->y;
	return (dx > 0 ? dx : -dx) + (dy > 0 ? dy : -dy);
}

static inline int state_cmp(const struct state *a, const struct state *b)
{
	return memcmp(a, b, sizeof(*a));
}

struct node
{
	struct state key;
	int type;		/* type of the node   */
	int distance;		/* distance up to now */
	int priority;		/* priority for the queue */

	struct node *next;	/* for the hashmap    */
};

struct heap
{
	struct node *pool;
	size_t count;
	size_t size;
};

struct visited
{
	struct node *table[HASH_SIZE];
	size_t count;
};

static void visited_free(struct visited *v)
{
	if (v)
	{
		for (size_t i = 0; i < HASH_SIZE; i++)
		{
			struct node *n = v->table[i];
			while (n)
			{
				struct node *t = n;
				n = n->next;
				free(t);
			}
		}
		free(v);
	}
}

static struct visited *visited_new(void)
{
	struct visited *v = calloc(1, sizeof(*v));
	if (v)
	{
		v->count = 0;
	}
	return v;
}

static struct node *visited_find(struct visited *v, const struct state *key)
{
	size_t idx = hashfn(key) % HASH_SIZE;
	struct node *n = v->table[idx];
	while (n && state_cmp(key, &n->key))
	{
		n = n->next;
	}
	return n;
}

static struct node *visited_add(struct visited *v, const struct state *key)
{
	struct node *n = visited_find(v, key);
	if (!n)
	{
		n = malloc(sizeof(*n));
		n->key = *key;
		assert(n);
		size_t idx = hashfn(key) % HASH_SIZE;
		n->next = v->table[idx];
		v->table[idx] = n;
	}
	return n;
}

static struct heap *heap_new(size_t initial)
{
	struct heap *h = malloc(sizeof(*h));
	if (!h)
	{
		return NULL;
	}

	h->count = 0;
	h->size = initial;
	h->pool = malloc(sizeof(h->pool[0]) * h->size);
	if (!h->pool)
	{
		free(h);
		return NULL;
	}

	return h;
}

static void heap_free(struct heap *h)
{
	if (h)
	{
		free(h->pool);
		free(h);
	}
}

static void heap_bubble_up(struct heap *h, size_t i)
{
	while (i > 0 && h->pool[(i-1)/2].priority > h->pool[i].priority)
	{
		struct node n = h->pool[i];
		h->pool[i] = h->pool[(i-1)/2];
		h->pool[(i-1)/2] = n;
		i = (i-1)/2;
	}
}

static inline int heap_push(struct heap *h, struct node n)
{
	if (h->count == h->size)
	{
		size_t newsize = h->size ? h->size * 2 : 2;
		struct node *newpool = realloc(h->pool, newsize * sizeof(newpool[0]));
		if (!newpool)
		{
			return -1;
		}
		h->size = newsize;
		h->pool = newpool;
	}

	h->pool[h->count] = n;
	heap_bubble_up(h, h->count);
	h->count++;
	return 0;
}

static void heap_bubble_down(struct heap *h, size_t i)
{
	for (;;)
	{
		size_t min = i;
		for (size_t j = i*2+1; j <= i*2+2; j++)
		{
			if (j < h->count && h->pool[j].priority < h->pool[min].priority)
			{
				min = j;
			}
		}

		if (min == i)
		{
			break;
		}

		struct node t = h->pool[i];
		h->pool[i] = h->pool[min];
		h->pool[min] = t;
		i = min;
	}
}

static struct node heap_pop(struct heap *h)
{
	struct node r = h->pool[0];
	h->count--;
	h->pool[0] = h->pool[h->count];
	heap_bubble_down(h, 0);
	return r;
}

static void map_free(struct map *m)
{
	if (m)
	{
		while (m->height-- > 0)
		{
			free(m->index[m->height]);
		}
		free(m->index);
		free(m);
	}
}

static inline void map_compute_index(struct map *m, size_t x0, size_t y0, size_t x1, size_t y1)
{
	for (size_t y = y0; y < y1; y++)
	{
		for (size_t x = x0; x < x1; x++)
		{
			int v;
			if (x == (size_t)m->target.x && y == (size_t)m->target.y)
			{
				v = m->depth;
			}
			else if (x == 0 || y == 0)
			{
				v = (m->depth + x * 16807 + y * 48271);
			}
			else
			{
				v = (m->depth + m->index[y][x-1] * m->index[y-1][x]);
			}
			m->index[y][x] = v % 20183;
		}
	}
}

static inline int map_get_risk(struct map *m, size_t x, size_t y)
{
	if (y >= m->height)
	{
		size_t newsize = y * 2;
		int **newi = realloc(m->index, newsize * sizeof(newi[0]));
		if (!newi)
		{
			abort();
		}

		m->index = newi;
 		for (size_t ny = m->height; ny < newsize; ny++)
		{
			m->index[ny] = malloc(m->width * sizeof(m->index[0][0]));
			if (!m->index[ny])
			{
				abort();
			}
		}

		map_compute_index(m, 0, m->height, m->width, newsize);
		m->height = newsize;
	}

	if (x >= m->width)
	{
		size_t newsize = x * 2;
		for (size_t ny = 0; ny < m->height; ny++)
		{
			int *newi = realloc(m->index[ny], newsize * sizeof(newi[0]));
			if (!newi)
			{
				abort();
			}
			m->index[ny] = newi;
		}
		map_compute_index(m, m->width, 0, newsize, m->height);
		m->width = newsize;
	}

	return m->index[y][x] % 3;
}

static struct map *map_load(FILE *input)
{
	struct map *m = malloc(sizeof(*m));
	if (!m)
	{
		return m;
	}

	if (fscanf(input, " depth: %d", &m->depth) != 1)
	{
		free(m);
		return NULL;
	}

	if (fscanf(input, " target: %d,%d", &m->target.x, &m->target.y) != 2)
	{
		free(m);
		return NULL;
	}
	m->target.tool = TORCH;
	m->width = m->height = 0;
	m->index = NULL;
	map_get_risk(m, m->target.x, m->target.y);

	return m;
}

static int map_rect_risk(struct map *m)
{
	int risk = 0;
	for (int y = 0; y < m->target.y+1; y++)
	{
		for (int x = 0; x < m->target.x+1; x++)
		{
			risk += m->index[y][x] % 3;
		}
	}
	return risk;
}

static int map_astar(struct map *m)
{
	static const int dx[] = {0, 0, 1, 0, -1};
	static const int dy[] = {0, -1, 0, 1, 0};

	struct heap *queue = heap_new(10000);
	assert(queue);

	struct visited *visited = visited_new();
	assert(visited);

	struct state start = {0, 0, TORCH};

	struct node *n = visited_add(visited, &start);
	n->type = map_get_risk(m, 0, 0);
	n->distance = 0;
	n->priority = 0;

	heap_push(queue, *n);
	while (queue->count)
	{
		struct node cur = heap_pop(queue);
		if (state_cmp(&cur.key, &m->target) == 0)
		{
			visited_free(visited);
			heap_free(queue);
			return cur.distance;
		}

		for (int i = 0; i < 5; i++)
		{
			struct state nstate = {
				.x = cur.key.x + dx[i],
				.y = cur.key.y + dy[i],
			};
			if (nstate.x < 0 || nstate.y < 0)
			{
				continue;
			}
			int type = map_get_risk(m, nstate.x, nstate.y);

			for (nstate.tool = 0; nstate.tool < 3; nstate.tool++)
			{
				if (state_cmp(&cur.key, &nstate) == 0
				    || type == nstate.tool
				    || cur.type == nstate.tool)
				{
					continue;
				}

				int newd = cur.distance;
				if (cur.key.x != nstate.x || cur.key.y != nstate.y)
				{
					newd++;
				}
				if (cur.key.tool != nstate.tool)
				{
					newd += 7;
				}

				n = visited_find(visited, &nstate);
				if (n && newd >= n->distance)
				{
					continue;
				}
				else if (!n)
				{
					n = visited_add(visited, &nstate);
				}

				n->type = type;
				n->distance = newd;
				n->priority = newd + manhattan(&nstate, &m->target);
				heap_push(queue, *n);
			}
		}
	}
	visited_free(visited);
	heap_free(queue);
	return 0;
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
	if (!m)
	{
		fprintf(stderr, "Cannot parse the map from the given file\n");
		return 1;
	}

	printf("Part1: %d\n", map_rect_risk(m));
	printf("Part2: %d\n", map_astar(m));
	map_free(m);
	return 0;
}
