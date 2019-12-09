#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct cart
{
	int id;
	char direction;
	size_t x, y;
	int i;
	int dead;
};

static void cart_init(struct cart *c, int id, size_t x, size_t y, int value)
{
	c->id = id;
	c->direction = value;
	c->x = x;
	c->y = y;
	c->i = 0;
	c->dead = 0;
}

static void cart_move(struct cart *c)
{
	switch (c->direction) {
	case '>': c->x++; break;
	case '<': c->x--; break;
	case '^': c->y--; break;
	case 'v': c->y++; break;
	}
}

static void cart_turn_left(struct cart *c)
{
	switch(c->direction) {
	case '^': c->direction = '<'; break;
	case '<': c->direction = 'v'; break;
	case 'v': c->direction = '>'; break;
	case '>': c->direction = '^'; break;
	}
}

static void cart_turn_right(struct cart *c)
{
	switch(c->direction) {
	case '^': c->direction = '>'; break;
	case '<': c->direction = '^'; break;
	case 'v': c->direction = '<'; break;
	case '>': c->direction = 'v'; break;
	}
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
			switch(dir) {
			default:
				continue;
			case 'v':
			case '^':
				line[i] = '|';
				break;
			case '>':
			case '<':
				line[i] = '-';
				break;
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

	/* realloc the lines to width */
	for (size_t i = 0; i < c->height; i++) {
		size_t len = strlen(c->data[i]);
		if (len != c->width) {
			char *newdata = realloc(c->data[i], c->width);
			if (newdata == NULL)
				abort();

			c->data[i] = newdata;
			memset(c->data[i] + len, 0, c->width - len);
		}
	}
	return c;
}

static void circuit_move_cart(struct circuit *ci, struct cart *ca)
{
	cart_move(ca);
	switch (ci->data[ca->y][ca->x]) {
	case '\\':
		if (ca->direction == '>')
			ca->direction = 'v';
		else if (ca->direction == '<')
			ca->direction = '^';
		else if (ca->direction == 'v')
			ca->direction = '>';
		else
			ca->direction = '<';
		break;

	case '/':
		if (ca->direction == '>')
			ca->direction = '^';
		else if (ca->direction == '<')
			ca->direction = 'v';
		else if (ca->direction == 'v')
			ca->direction = '<';
		else
			ca->direction = '>';
		break;

	case '-':
	case '|':
		break;

	case '+':
		if (ca->i == 0) {
			cart_turn_left(ca);
		} else if (ca->i == 1) {
			/* go straight */
		} else {
			cart_turn_right(ca);
		}
		ca->i = (ca->i + 1) % 3;
		break;

	}
}

static int cart_cmp(const void *ca, const void *cb)
{
	const struct cart *a = ca, *b = cb;
	if (a->y != b->y)
		return a->y - b->y;
	return a->x - b->x;
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
			    c->carts[j].x == c->carts[i].x &&
			    c->carts[j].y == c->carts[i].y) {
				c->carts[i].dead = 1;
				c->carts[j].dead = 1;
				crash = 1;
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
						printf("Cart %d crashed at %zu,%zu\n",
						       c->carts[i].id,
						       c->carts[i].x,
						       c->carts[i].y);
					}
				}
			}
			circuit_remove_dead(c);
		}
		if (c->ccount) {
			printf("Survived cart: %d at %zu,%zu\n",
			       c->carts[0].id,
			       c->carts[0].x,
			       c->carts[0].y);
		}
		printf("Last tick %d\n", i);
	}

	circuit_free(c);
	return -1;
}
