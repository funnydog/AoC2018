#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct vec
{
	int x;
	int y;
};

struct cart
{
	struct vec p;
	enum { UP, RIGHT, DOWN, LEFT } d;
	enum { TURN_LEFT, FORWARD, TURN_RIGHT } t;
};

struct map
{
	char **data;
	size_t height;
	size_t dsize;

	struct cart *carts;
	size_t count;
	size_t size;
};

static void cart_init(struct cart *c, int x, int y, int value)
{
	c->p.x = x;
	c->p.y = y;
	switch (value)
	{
	case '^': c->d = UP; break;
	case '>': c->d = RIGHT; break;
	case 'v': c->d = DOWN; break;
	case '<': c->d = LEFT; break;
	}
	c->t = TURN_LEFT;
}

static int cart_cmp(const struct cart *a, const struct cart *b)
{
	if (a->p.y != b->p.y)
	{
		return a->p.y - b->p.y;
	}
	return a->p.x - b->p.x;
}

static void cart_update(struct cart *c, struct map *m)
{
	static const int dx[] = {0, 1, 0, -1};
	static const int dy[] = {-1, 0, 1, 0};
	static const int sl[] = {RIGHT, UP, LEFT, DOWN};
	static const int bs[] = {LEFT, DOWN, RIGHT, UP};
	c->p.x += dx[c->d];
	c->p.y += dy[c->d];

	assert(0 <= c->p.y && (size_t)c->p.y < m->height);
	switch (m->data[c->p.y][c->p.x])
	{
	case '+':
		switch (c->t)
		{
		case TURN_LEFT:
			c->d = (c->d + 3) % 4;
			c->t = FORWARD;
			break;

		case FORWARD:
			c->t = TURN_RIGHT;
			break;

		case TURN_RIGHT:
			c->t = TURN_LEFT;
			c->d = (c->d + 1) % 4;
			break;
		}
		break;
	case '\\':
		c->d = bs[c->d];
		break;
	case '/':
		c->d = sl[c->d];
		break;
	default:
		break;
	}
}

static void cart_bubble_up(struct cart *carts, size_t pos)
{
	while (pos > 0)
	{
		size_t parent = (pos-1)/2;
		if (cart_cmp(carts+parent, carts+pos) < 0)
		{
			break;
		}

		/* swap */
		struct cart tmp = carts[parent];
		carts[parent] = carts[pos];
		carts[pos] = tmp;
		pos = parent;
	}
}

static void cart_bubble_down(struct cart *carts, size_t size, size_t pos)
{
	for(;;)
	{
		size_t min = pos;
		for (size_t i = pos*2+1; i<=pos*2+2; i++)
		{
			if (i < size && cart_cmp(carts+i, carts+min) < 0)
			{
				min = i;
			}
		}

		if (min == pos)
		{
			break;
		}

		/* swap */
		struct cart t = carts[min];
		carts[min] = carts[pos];
		carts[pos] = t;

		pos = min;
	}
}

static int map_push_cart(struct map *m, int x, int y, int v)
{
	if (m->count == m->size)
	{
		size_t size = m->size ? m->size * 2 : 2;
		struct cart *n = realloc(m->carts, size * sizeof(*n));
		if (!n)
		{
			return 0;
		}
		m->size = size;
		m->carts = n;
	}
	cart_init(m->carts + m->count, x, y, v);
	cart_bubble_up(m->carts, m->count);
	m->count++;
	return 1;
}

static struct cart map_pop_cart(struct map *m)
{
	assert(m->count);
	struct cart t = m->carts[0];
	m->carts[0] = m->carts[--m->count];
	cart_bubble_down(m->carts, m->count, 0);
	return t;
}

static void map_free(struct map *m)
{
	if (m)
	{
		while(m->height-->0)
		{
			free(m->data[m->height]);
		}
		free(m->data);
		free(m->carts);
		free(m);
	}
}

static struct map *map_load(FILE *input)
{
	struct map *m = calloc(1, sizeof(*m));
	if (!m)
	{
		return m;
	}

	size_t sline = 0;
	char *line = NULL;
	while (getline(&line, &sline, input) != -1)
	{
		/* add the carts and remove them from the map */
		for (size_t x = 0; line[x] != 0; x++)
		{
			if (!strchr("^>v<", line[x]))
			{
				continue;
			}
			else if (map_push_cart(m, x, m->height, line[x]))
			{
				line[x] = (line[x] == 'v' || line[x] == '^') ? '|' : '-';
			}
		}

		/* add the line in the map */
		if (m->height == m->dsize)
		{
			size_t dsize = m->dsize ? m->dsize * 2 : 2;
			char **data = realloc(m->data, dsize * sizeof(*data));
			if (!data)
			{
				map_free(m);
				return NULL;
			}
			m->dsize = dsize;
			m->data = data;
		}
		m->data[m->height++] = line;
		line = NULL;
		sline = 0;
	}
	free(line);
	return m;
}

static int find_vec(const struct vec *parr, size_t psize, struct vec p)
{
	for (size_t i = 0; i < psize; i++)
	{
		if (parr[i].x == p.x && parr[i].y == p.y)
		{
			return i;
		}
	}
	return -1;
}

static void map_simulate(struct map *m, struct vec *part1, struct vec *part2)
{
	/* array of the cart positions */
	struct vec *cartmap = malloc(m->count * sizeof(*cartmap));
	assert(cartmap);
	for (size_t i = 0; i < m->count; i++)
	{
		cartmap[i] = m->carts[i].p;
	}
	size_t cartmapcount = m->count;

	/* priority queue for next tick */
	struct cart *cheap = malloc(m->count * sizeof(*cheap));
	size_t cheapcount = 0;
	assert(cheap);

	/* simulation */
	size_t left = m->count;
	int first = 0;
	while (left > 1)
	{
		while (m->count)
		{
			struct cart c = map_pop_cart(m);

			/* if the cart already crashed its position is
			 * not in the cartmap */
			int i = find_vec(cartmap, cartmapcount, c.p);
			if (i < 0)
			{
				continue;
			}

			/* remove the cart position from the cartmap
			 * and update the cart */
			cartmap[i] = cartmap[--cartmapcount];
			cart_update(&c, m);

			/* check if the cart has crashed */
			i = find_vec(cartmap, cartmapcount, c.p);
			if (i < 0)
			{
				/* add the new position to the cartmap */
				cartmap[cartmapcount++] = c.p;

				/* add the cart to the next tick's queue */
				cheap[cheapcount] = c;
				cart_bubble_up(cheap, cheapcount);
				cheapcount++;
			}
			else
			{
				/* crash occurred */
				cartmap[i] = cartmap[--cartmapcount];
				left -= 2;
				if (!first)
				{
					first = 1;
					*part1 = c.p;
				}
			}
		}

		/* swap the cart queues */
		struct cart *t = m->carts;
		m->carts = cheap;
		cheap = t;
		m->count = cheapcount;
		cheapcount = 0;
	}

	/* find the first non crashed cart */
	while (m->count)
	{
		struct cart c = map_pop_cart(m);
		if (find_vec(cartmap, cartmapcount, c.p) != -1)
		{
			*part2 = c.p;
			break;
		}
	}

	free(cheap);
	free(cartmap);
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
		fprintf(stderr, "Cannot parse the data\n");
		return 1;
	}

	struct vec part1 = {0}, part2 = {0};
	map_simulate(m, &part1, &part2);
	map_free(m);
	printf("Part1: %d,%d\n", part1.x, part1.y);
	printf("Part2: %d,%d\n", part2.x, part2.y);
	return 0;
}
