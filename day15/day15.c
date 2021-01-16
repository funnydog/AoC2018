#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const int dx[] = {0,-1,1,0};
static const int dy[] = {-1,0,0,1};

struct unit
{
	int class;
	int hp;
	int attack;
	size_t x, y;
};

struct map
{
	char **data;
	size_t height;
	size_t width;
	size_t dsize;

	struct unit *units;
	size_t ucount;
	size_t usize;

	size_t elves;
	size_t goblins;

	int **dist;
};

static void map_free(struct map *g)
{
	if (g)
	{
		free(g->units);
		while (g->height-- > 0)
		{
			free(g->dist[g->height]);
			free(g->data[g->height]);
		}
		free(g->dist);
		free(g->data);
		free(g);
	}
}

static struct map *map_copy(const struct map *g)
{
	struct map *c = calloc(1, sizeof(*c));
	if (c)
	{
		c->dsize = c->height = g->height;
		c->width = g->width;
		c->usize = c->ucount = g->ucount;
		c->elves = g->elves;
		c->goblins = g->goblins;

		c->data = calloc(c->dsize, sizeof(c->data[0]));
		c->dist = calloc(c->dsize, sizeof(c->dist[0]));
		c->units = calloc(c->usize, sizeof(c->units[0]));
		if (!c->data || !c->dist || !c->units)
		{
			map_free(c);
			return NULL;
		}

		memmove(c->units, g->units, c->ucount * sizeof(c->units[0]));
		for (size_t i = 0; i < c->height; i++)
		{
			c->data[i] = strdup(g->data[i]);
			c->dist[i] = malloc(c->width * sizeof(c->dist[0][0]));
			if (!c->data[i] || !c->dist[i])
			{
				map_free(c);
				return NULL;
			}
			memmove(c->dist[i], g->dist[i], c->width * sizeof(c->dist[0][0]));
		}
	}
	return c;
}

static void map_bfs(struct map *g, struct unit *u)
{
	/* reset the map */
	for (size_t y = 0; y < g->height; y++)
	{
		for (size_t x = 0; x < g->width; x++)
		{
			g->dist[y][x] = INT_MAX;
		}
	}

	struct pos { size_t x, y; } queue[1024];
	size_t wi = 0, ri = 0;
	queue[wi++ & 1023] = (struct pos){u->x, u->y};
	g->dist[u->y][u->x] = 0;
	while (wi != ri)
	{
		struct pos p = queue[ri++ & 1023];
		for (int i = 0; i < 4; i++)
		{
			size_t x = p.x + dx[i];
			size_t y = p.y + dy[i];

			if (g->data[y][x] != '.')
			{
				continue;
			}

			if (g->dist[y][x] == INT_MAX)
			{
				g->dist[y][x] = g->dist[p.y][p.x] + 1;
				if (wi-ri == 1024)
				{
					abort();
				}
				queue[wi++ & 1023] = (struct pos){x, y};
			}
		}
	}
}

static void map_move_unit(struct map *g, struct unit *u)
{
	/* check if an enemy is already in range */
	int enemy = u->class == 'E' ? 'G' : 'E';
	for (int i = 0; i < 4; i++)
	{
		if (g->data[u->y + dy[i]][u->x + dx[i]] == enemy)
		{
			return;
		}
	}

	/* find a destination to move the unit to */
	map_bfs(g, u);
	int min = INT_MAX;
	size_t tx, ty;
	for (struct unit *e = g->units; e < g->units + g->ucount; e++)
	{
		if (u->class == e->class || e->hp <= 0)
		{
			continue;
		}
		for (int i = 0; i < 4; i++)
		{
			size_t x = e->x + dx[i];
			size_t y = e->y + dy[i];
			if (min > g->dist[y][x])
			{
				min = g->dist[y][x];
				tx = x;
				ty = y;
			}
		}
	}

	/* no suitable destination found */
	if (min == INT_MAX)
	{
		return;
	}

	/* walk the path back until we find the place to move to */
	for (;;)
	{
		size_t mx = tx;
		size_t my = ty;
		for (int i = 0; i < 4; i++)
		{
			size_t x = tx + dx[i];
			size_t y = ty + dy[i];
			if (min > g->dist[y][x])
			{
				min = g->dist[y][x];
				my = y;
				mx = x;
			}
		}
		if (mx == u->x && my == u->y)
		{
			break;
		}
		tx = mx;
		ty = my;
	}

	/* update the map and the unit position */
	g->data[u->y][u->x] = '.';
	g->data[ty][tx] = u->class;
	u->x = tx;
	u->y = ty;
}

static int map_attack_unit(struct map *g, struct unit *u)
{
	int minhp = INT_MAX;
	struct unit *target = NULL;
	for (int i = 0; i < 4; i++)
	{
		for (struct unit *e = g->units; e < g->units + g->ucount; e++)
		{
			/* don't attack friends or dead units */
			if (e->class == u->class || e->hp <= 0)
			{
				continue;
			}

			if (u->x + dx[i] == e->x && u->y + dy[i] == e->y
			    && minhp > e->hp)
			{
				minhp = e->hp;
				target = e;
			}
		}
	}

	if (target)
	{
		target->hp -= u->attack;
		if (target->hp <= 0)
		{
			g->data[target->y][target->x] = '.';
			return 1;
		}
	}

	return 0;
}

static int unit_cmp(const void *pa, const void *pb)
{
	const struct unit *a = pa;
	const struct unit *b = pb;
	if (a->y != b->y)
	{
		return a->y - b->y;
	}
	return a->x - b->x;
}

static int map_points(struct map *g)
{
	int count = 0;
	for (struct unit *u = g->units; u < g->units + g->ucount; u++)
	{
		if (u->hp > 0)
		{
			count += u->hp;
		}
	}
	return count;
}

static void map_print(struct map *g)
{
	size_t i = 0;
	for (size_t y = 0; y < g->height; y++)
	{
		printf("%s", g->data[y]);
		for (; i < g->ucount && g->units[i].y == y; i++)
		{
			if (g->units[i].hp > 0)
			{
				printf(" %c(%d)", g->units[i].class, g->units[i].hp);
			}
		}
		printf("\n");
	}
}

static int map_simulate(struct map *g, int retearly)
{
	int rounds = 0;
	for (rounds = 0; ; rounds++)
	{
		/* sort the units */
		qsort(g->units, g->ucount, sizeof(g->units[0]), unit_cmp);

		/* play a round */
		for (struct unit *u = g->units; u < g->units + g->ucount; u++)
		{
			if (u->hp <= 0)
			{
				continue;
			}

			map_move_unit(g, u);
			if (map_attack_unit(g, u))
			{
				if (u->class == 'G')
				{
					if (retearly || --g->elves == 0)
					{
						return rounds;
					}
				}
				else if (--g->goblins == 0)
				{
					return rounds;
				}
			}
		}
	}
}

static struct map *map_load(FILE *input)
{
	struct map *g = calloc(1, sizeof(*g));
	if (!g)
	{
		return NULL;
	}

	char *line = NULL;
	size_t linesize = 0;
	while (getline(&line, &linesize, input) != -1)
	{
		for (char *t = line; *t; t++)
		{
			int class;
			switch(*t)
			{
			case 'G':
				class = *t;
				g->goblins++;
				break;

			case 'E':
				class = *t;
				g->elves++;
				break;

			case '\n':
				g->width = (size_t)(t - line);
				*t = 0;
				continue;

			default:
				continue;
			}
			if (g->ucount == g->usize)
			{
				size_t newsize = g->usize ? g->usize * 2 : 2;
				struct unit *newunits = realloc(g->units, newsize * sizeof(*newunits));
				if (newunits == NULL)
				{
					map_free(g);
					return NULL;
				}
				g->usize = newsize;
				g->units = newunits;
			}
			g->units[g->ucount].hp = 200;
			g->units[g->ucount].attack = 3;
			g->units[g->ucount].class = class;
			g->units[g->ucount].x = (size_t)(t - line);
			g->units[g->ucount].y = g->height;
			g->ucount++;
		}

		if (g->height == g->dsize)
		{
			size_t newsize = g->dsize ? g->dsize * 2 : 2;
			char **newdata = realloc(g->data, newsize * sizeof(*newdata));
			if (newdata == NULL)
			{
				map_free(g);
				return NULL;
			}
			g->dsize = newsize;
			g->data = newdata;

			int **newdist = realloc(g->dist, newsize * sizeof(*newdist));
			if (newdist == NULL)
			{
				map_free(g);
				return NULL;
			}
			g->dist = newdist;
		}
		g->data[g->height] = strdup(line);
		g->dist[g->height] = malloc(g->width * sizeof(int));
		if (g->data[g->height] == NULL || g->dist[g->height] == NULL)
		{
			free(g->dist[g->height]);
			free(g->data[g->height]);
			map_free(g);
			return NULL;
		}
		g->height++;
	}
	free(line);

	return g;
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

	struct map *g = map_load(input);
	fclose(input);

	printf("Read a map of size %zu,%zu, units %zu\n", g->width, g->height, g->ucount);
	printf("Answer1:\n");
	struct map *c = map_copy(g);
	int rounds = map_simulate(c, 0);
	map_print(c);
	int points = map_points(c);
	printf("Rounds %d, points %d\n", rounds, points);
	printf("Outcome of the battle %d with attack level = 3\n", rounds * points);
	map_free(c);

	/* find the minimum attack level by bisecting */
	int low = 4, high = 200;
	while (low < high)
	{
		int attack = low + (high - low) / 2;
		printf("Trying with attack level %d...\n", attack);

		c = map_copy(g);
		for (struct unit *u = c->units; u < c->units + c->ucount; u++)
		{
			if (u->class == 'E')
			{
				u->attack = attack;
			}
		}
		map_simulate(c, 1);
		if (c->goblins)
		{
			low = attack + 1;
		}
		else
		{
			high = attack;
		}
		map_free(c);
	}
	for (struct unit *u = g->units; u < g->units + g->ucount; u++)
	{
		if (u->class == 'E')
		{
			u->attack = high;
		}
	}
	rounds = map_simulate(g, 0);
	map_print(g);
	points = map_points(g);
	printf("Minimum attack level: %d\n", high);
	printf("Rounds %d, points %d\n", rounds, points);
	printf("Outcome of the battle %d with attack level = %d\n", rounds * points, high);

	map_free(g);
	return 0;
}
