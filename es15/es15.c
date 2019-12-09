#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
	CLASS_ELF,
	CLASS_GOBLIN,
};

const int off[][2] = {
	/* X,  Y */
	{  0, -1 },
	{ -1,  0 },
	{  1,  0 },
	{  0,  1 },
};

struct unit
{
	int class;
	int hitpoints;
	int attack;
	size_t x, y;
};

struct playground
{
	char **data;
	size_t height;
	size_t width;
	size_t dsize;

	struct unit *units;
	size_t ucount;
	size_t usize;

	int **dist;
};

static void playground_free(struct playground *g)
{
	if (g) {
		free(g->units);
		while (g->height-- > 0) {
			free(g->dist[g->height]);
			free(g->data[g->height]);
		}
		free(g->dist);
		free(g->data);
		free(g);
	}
}

static struct playground *playground_copy(struct playground *g)
{
	struct playground *c = calloc(1, sizeof(*c));
	if (c) {
		c->dsize = c->height = g->height;
		c->width = g->width;
		c->usize = c->ucount = g->ucount;

		c->data = calloc(c->dsize, sizeof(c->data[0]));
		c->dist = calloc(c->dsize, sizeof(c->dist[0]));
		c->units = calloc(c->usize, sizeof(c->units[0]));
		if (!c->data || !c->dist || !c->units) {
			playground_free(c);
			return NULL;
		}

		memmove(c->units, g->units, c->ucount * sizeof(c->units[0]));
		for (size_t i = 0; i < c->height; i++) {
			c->data[i] = strdup(g->data[i]);
			c->dist[i] = malloc(c->width * sizeof(c->dist[0][0]));
			if (!c->data[i] || !c->dist[i]) {
				playground_free(c);
				return NULL;
			}
			memmove(c->dist[i], g->dist[i], c->width * sizeof(c->dist[0][0]));
		}
	}
	return c;
}

static void playground_bfs(struct playground *g, struct unit *u)
{
	/* reset the map */
	for (size_t y = 0; y < g->height; y++)
		for (size_t x = 0; x < g->width; x++)
			g->dist[y][x] = INT_MAX;

	/* mark the units as walls in the map */
	for (struct unit *e = g->units; e < g->units + g->ucount; e++) {
		if (e->hitpoints > 0 && u != e)
			g->data[e->y][e->x] = '#';
	}

	struct pos {
		size_t x, y;
	} queue[1000];
	size_t qcount = 0;
	queue[qcount++] = (struct pos){u->x, u->y};
	g->dist[u->y][u->x] = 0;
	while (qcount > 0) {
		struct pos p = queue[0];
		qcount--;
		memmove(queue, queue+1, qcount*sizeof(queue[0]));
		for (int i = 0; i < 4; i++) {
			size_t x = p.x + off[i][0];
			size_t y = p.y + off[i][1];

			if (g->data[y][x] != '.')
				continue;

			if (g->dist[y][x] == INT_MAX) {
				g->dist[y][x] = g->dist[p.y][p.x] + 1;
				if (qcount == 1000)
					abort();
				queue[qcount++] = (struct pos){x, y};
			}
		}
	}

	/* remove the units from the map */
	for (struct unit *e = g->units; e < g->units + g->ucount; e++) {
		if (e->hitpoints > 0 && u != e)
			g->data[e->y][e->x] = '.';
	}
}

static int playground_find_target(struct playground *g, struct unit *u, size_t *px, size_t *py)
{
	playground_bfs(g, u);
	int min = INT_MAX;
	for (struct unit *e = g->units; e < g->units + g->ucount; e++) {
		if (u->class == e->class)
			continue;

		if (e->hitpoints <= 0)
			continue;

		for (int i = 0; i < 4; i++) {
			size_t ex = e->x + off[i][0];
			size_t ey = e->y + off[i][1];
			if (min > g->dist[ey][ex]) {
				min = g->dist[ey][ex];
				*px = ex;
				*py = ey;
			}
		}
	}

	return min != INT_MAX;
}

static int playground_move_unit(struct playground *g, struct unit *u)
{
	struct unit *e;
	for (e = g->units; e < g->units + g->ucount; e++) {
		if (e->hitpoints > 0 && e->class != u->class)
			break;
	}
	if (e == g->units + g->ucount)
		return 0;

	size_t tx, ty;
	if (!playground_find_target(g, u, &tx, &ty))
		return 1;

	if (tx == u->x && ty == u->y)
		return 1;

	for (;;) {
		int min = INT_MAX;
		size_t minx = tx;
		size_t miny = ty;
		for (int i = 0; i < 4; i++) {
			size_t x = tx + off[i][0];
			size_t y = ty + off[i][1];
			if (min > g->dist[y][x]) {
				min = g->dist[y][x];
				minx = x;
				miny = y;
			}
		}
		if (minx == u->x && miny == u->y)
			break;

		tx = minx;
		ty = miny;
	}

	u->x = tx;
	u->y = ty;
	return 1;
}

static int playground_attack_unit(struct playground *g, struct unit *u)
{
	struct unit *mine = NULL;
	int minhp = INT_MAX;
	for (int i = 0; i < 4; i++) {
		for (struct unit *e = g->units; e < g->units + g->ucount; e++) {
			/* don't attack friends */
			if (e->class == u->class)
				continue;

			/* don't attack dead units */
			if (e->hitpoints <= 0)
				continue;

			if (u->x + off[i][0] == e->x &&
			    u->y + off[i][1] == e->y &&
			    minhp > e->hitpoints) {
				minhp = e->hitpoints;
				mine = e;
			}
		}
	}

	if (mine) {
		mine->hitpoints -= u->attack;
		return 1;
	}

	return 0;
}

static int unit_cmp(const void *pa, const void *pb)
{
	const struct unit *a = pa;
	const struct unit *b = pb;
	if (a->y != b->y)
		return a->y - b->y;

	return a->x - b->x;
}

static int playground_points(struct playground *g)
{
	int count = 0;
	for (struct unit *u = g->units; u < g->units + g->ucount; u++) {
		if (u->hitpoints > 0)
			count += u->hitpoints;
	}
	return count;
}

static void playground_print(struct playground *g)
{
	for (size_t y = 0; y < g->height; y++) {
		for (size_t x = 0; x < g->width; x++) {
			struct unit *u;
			for (u = g->units; u < g->units + g->ucount; u++) {
				if (u->hitpoints > 0 && x == u->x && y == u->y) {
					printf(" %c", u->class == CLASS_ELF ? 'E' : 'G');
					break;
				}
			}
			if (u == g->units + g->ucount) {
				printf(" %c", g->data[y][x]);
			}
		}

		for (struct unit *u = g->units; u < g->units + g->ucount; u++) {
			if (u->hitpoints > 0 && u->y == y) {
				printf(" %c(%d)", u->class == CLASS_ELF ? 'E' : 'G', u->hitpoints);
			}
		}
		printf("\n");
	}
}

static int playground_simulate(struct playground *c, int retearly)
{
	int rounds = 0;
	for (rounds = 0; ; rounds++) {
		/* sort the units */
		qsort(c->units, c->ucount, sizeof(c->units[0]), unit_cmp);

		/* move and attack */
		for (struct unit *u = c->units; u < c->units + c->ucount; u++) {
			if (u->hitpoints > 0) {
				if (!playground_move_unit(c, u))
					return rounds;

				playground_attack_unit(c, u);
			}
		}

		/* check if elf dead */
		if (retearly) {
			for (struct unit *u = c->units; u < c->units + c->ucount; u++)
				if  (u->class == CLASS_ELF && u->hitpoints <= 0)
					return rounds;
		}
	}
}

static struct playground *playground_load(FILE *input)
{
	struct playground *g = calloc(1, sizeof(*g));
	if (!g)
		return NULL;

	char *line = NULL;
	size_t linesize = 0;
	while (getline(&line, &linesize, input) != -1) {
		for (char *t = line; *t; t++) {
			int class;
			switch(*t) {
			case 'G': class = CLASS_GOBLIN; break;
			case 'E': class = CLASS_ELF; break;
			case '\n':
				g->width = (size_t)(t - line);
				*t = 0;
				continue;
			default:
				continue;
			}
			if (g->ucount == g->usize) {
				size_t newsize = g->usize ? g->usize * 2 : 2;
				struct unit *newunits = realloc(g->units, newsize * sizeof(*newunits));
				if (newunits == NULL) {
					playground_free(g);
					return NULL;
				}
				g->usize = newsize;
				g->units = newunits;
			}
			g->units[g->ucount].hitpoints = 200;
			g->units[g->ucount].attack = 3;
			g->units[g->ucount].class = class;
			g->units[g->ucount].x = (size_t)(t - line);
			g->units[g->ucount].y = g->height;
			g->ucount++;
			*t = '.';
		}

		if (g->height == g->dsize) {
			size_t newsize = g->dsize ? g->dsize * 2 : 2;
			char **newdata = realloc(g->data, newsize * sizeof(*newdata));
			if (newdata == NULL) {
				playground_free(g);
				return NULL;
			}
			g->dsize = newsize;
			g->data = newdata;

			int **newdist = realloc(g->dist, newsize * sizeof(*newdist));
			if (newdist == NULL) {
				playground_free(g);
				return NULL;
			}
			g->dist = newdist;
		}
		g->data[g->height] = strdup(line);
		g->dist[g->height] = malloc(g->width * sizeof(int));
		if (g->data[g->height] == NULL || g->dist[g->height] == NULL) {
			free(g->dist[g->height]);
			free(g->data[g->height]);
			playground_free(g);
			return NULL;
		}
		g->height++;
	}
	free(line);

	return g;
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

	struct playground *g = playground_load(input);
	fclose(input);

	printf("Read a map of size %zu,%zu, units %zu\n", g->width, g->height, g->ucount);
	printf("Answer1:\n");
	struct playground *c = playground_copy(g);
	int rounds = playground_simulate(c, 0);
	playground_print(c);
	int points = playground_points(c);
	printf("Rounds %d, points %d\n", rounds, points);
	printf("Outcome of the battle %d with attack level = 3\n", rounds * points);
	playground_free(c);

	/* find the minimum attack level by bisecting */
	c = NULL;
	int low = 4, high = 200;
	while (low < high) {
		playground_free(c);
		c = playground_copy(g);
		int attack = (low + high) / 2;
		for (struct unit *u = c->units; u < c->units + c->ucount; u++)
			if (u->class == CLASS_ELF)
				u->attack = attack;

		printf("Trying with attack level %d...\n", attack);
		rounds = playground_simulate(c, 1);
		points = playground_points(c);
		struct unit *u;
		for (u = c->units; u < c->units + c->ucount; u++)
			if (u->class == CLASS_ELF && u->hitpoints <= 0)
				break;
		if (u < c->units + c->ucount) {
			low = attack + 1;
		} else {
			high = attack;
		}
	}
	playground_print(c);
	playground_free(c);
	for (struct unit *u = g->units; u < g->units + g->ucount; u++)
		if (u->class == CLASS_ELF)
			u->attack = high;
	rounds = playground_simulate(g, 0);
	points = playground_points(g);
	printf("Minimum attack level: %d\n", high);
	printf("Rounds %d, points %d\n", rounds, points);
	printf("Outcome of the battle %d with attack level = %d\n", rounds * points, high);

	playground_free(g);
	return 0;
}
