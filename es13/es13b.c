#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct vector
{
	int x;
	int y;
};

static struct vector xmul(struct vector a, struct vector b)
{
	struct vector r = {
		.x = a.x * b.x - a.y * b.y,
		.y = a.x * b.y + a.y * b.x,
	};
	return r;
}

static struct vector add(struct vector a, struct vector b)
{
	struct vector r = {
		.x = a.x + b.x,
		.y = a.y + b.y,
	};
	return r;
}

static int equal(struct vector a, struct vector b)
{
	return a.x == b.x && a.y == b.y;
}

enum {
	TURN_LEFT,
	GO_STRAIGHT,
	TURN_RIGHT,
};

struct cart
{
	int id;
	struct vector pos;
	struct vector vel;
	int state;
	int dead;
};

static void cart_init(struct cart *c, int id, int x, int y, int value)
{
	c->id = id;
	c->pos.x = x;
	c->pos.y = y;
	c->state = TURN_LEFT;
	switch (value) {
	case '^': c->vel = (struct vector){ 0, -1}; break;
	case 'v': c->vel = (struct vector){ 0,  1}; break;
	case '<': c->vel = (struct vector){-1,  0}; break;
	case '>': c->vel = (struct vector){ 1,  0}; break;
	default:
		abort();
	}
}

static void cart_move(struct cart *c)
{
	c->pos = add(c->pos, c->vel);
}

static void cart_turn_left(struct cart *c)
{
	c->vel = xmul(c->vel, (struct vector){0, -1});
}

static void cart_turn_right(struct cart *c)
{
	c->vel = xmul(c->vel, (struct vector){0, 1});
}

struct circuit
{
	char **data;
	size_t height;
	size_t size;
	size_t width;

	struct cart *carts;
	size_t ccount;
	size_t csize;
};

static void circuit_free(struct circuit *c)
{
	if (c) {
		while (c->height-- > 0)
			free(c->data[c->height]);

		free(c->data);
		free(c->carts);
		free(c);
	}
}

struct circuit *circuit_load(FILE *input)
{
	struct circuit *c = calloc(1, sizeof(*c));
	if (!c)
		return NULL;

	char *line = NULL;
	size_t linesize = 0;
	while (getline(&line, &linesize, input) != -1) {
		if (c->height == c->size) {
			size_t newsize = c->size ? c->size * 2 : 2;
			char **newdata = realloc(c->data, newsize * sizeof(newdata[0]));
			if (newdata == NULL)
				break;
			c->data = newdata;
			c->size = newsize;
		}
		size_t linelen = strlen(line)-1;
		line[linelen] = 0;
		if (c->width < linelen) {
			c->width = linelen;
		}
		/* find the carts */
		for (int i = 0; line[i]; i++) {
			int dir = line[i];
			if (dir == 'v' || dir == '^') {
				line[i] = '|';
			} else if (dir == '>' || dir == '<') {
				line[i] = '-';
			} else {
				continue;
			}
			if (c->ccount == c->csize) {
				size_t newsize = c->csize ? c->csize * 2 : 2;
				struct cart *newcarts = realloc(
					c->carts, newsize * sizeof(newcarts[0]));
				if (newcarts == NULL)
					break;
				c->carts = newcarts;
				c->csize = newsize;
			}
			cart_init(c->carts + c->ccount, c->ccount, i, c->height, dir);
			c->ccount++;
		}
		c->data[c->height] = strdup(line);
		c->height++;
	}
	free(line);

	return c;
}

static void circuit_move_cart(struct circuit *ci, struct cart *ca)
{
	cart_move(ca);
	switch (ci->data[ca->pos.y][ca->pos.x]) {
	case '\\':
		if (ca->vel.x == 0) {
			cart_turn_left(ca);
		} else {
			cart_turn_right(ca);
		}
		break;

	case '/':
		if (ca->vel.x == 0) {
			cart_turn_right(ca);
		} else {
			cart_turn_left(ca);
		}
		break;

	case '+':
		if (ca->state == TURN_LEFT) {
			cart_turn_left(ca);
			ca->state = GO_STRAIGHT;
		} else if (ca->state == GO_STRAIGHT) {
			ca->state = TURN_RIGHT;
		} else {
			cart_turn_right(ca);
			ca->state = TURN_LEFT;
		}
		break;

	}
}

static int cart_cmp(const void *ca, const void *cb)
{
	const struct cart *a = ca, *b = cb;
	if (a->pos.y != b->pos.y)
		return a->pos.y - b->pos.y;

	return a->pos.x - b->pos.x;
}

static int circuit_move_all(struct circuit *c)
{
	int crash = 0;
	for (size_t i = 0; i < c->ccount; i++) {
		if (c->carts[i].dead)
			continue;

		circuit_move_cart(c, c->carts+i);
		for (size_t j = 0; j < c->ccount; j++) {
			if (i != j && !c->carts[j].dead &&
			    equal(c->carts[i].pos, c->carts[j].pos)) {
				c->carts[i].dead = 1;
				c->carts[j].dead = 1;
				crash = 1;
				break;
			}
		}
	}
	qsort(c->carts, c->ccount, sizeof(struct cart), cart_cmp);
	return crash;
}

static void circuit_remove_dead(struct circuit *c)
{
	for (size_t i = 0; i < c->ccount;) {
		if (c->carts[i].dead) {
			c->ccount--;
			memmove(c->carts+i, c->carts+i+1,
				(c->ccount-i) * sizeof(struct cart));
		} else {
			i++;
		}
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

	struct circuit *c = circuit_load(input);
	fclose(input);

	if (c != NULL) {
		printf("Read circuit of %zu rows, %zu carts\n", c->height, c->ccount);
		int i;
		for (i = 0; c->ccount > 1; i++) {
			if (circuit_move_all(c)) {
				for (size_t i = 0; i < c->ccount; i++) {
					if (c->carts[i].dead) {
						printf("Cart %2d crashed at %3d,%3d\n",
						       c->carts[i].id,
						       c->carts[i].pos.x,
						       c->carts[i].pos.y);
					}
				}
			}
			circuit_remove_dead(c);
		}
		if (c->ccount) {
			printf("Survived cart: %2d at %3d,%3d\n",
			       c->carts[0].id,
			       c->carts[0].pos.x,
			       c->carts[0].pos.y);
		}
		printf("Last tick %d\n", i);
	}

	circuit_free(c);
	return -1;
}
