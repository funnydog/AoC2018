#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct bot
{
	int64_t x, y, z, r;
};

struct botset
{
	struct bot *pool;
	size_t count;
	size_t size;
	size_t strongest;
};

static void botset_free(struct botset *bs)
{
	if (bs)
	{
		free(bs->pool);
		free(bs);
	}
}

static struct botset *botset_load(FILE *input)
{
	struct botset *bs = calloc(1, sizeof(*bs));
	if (!bs)
	{
		return NULL;
	}

	int64_t maxr = INT64_MIN;
	struct bot t;
	while (fscanf(input, " pos=<%ld,%ld,%ld>, r=%ld", &t.x, &t.y, &t.z, &t.r) == 4)
	{
		/* find the strongest */
		if (maxr < t.r)
		{
			maxr = t.r;
			bs->strongest = bs->count;
		}

		/* allocate enough space */
		if (bs->count == bs->size)
		{
			size_t newsize = bs->size ? bs->size * 2 : 2;
			struct bot *newpool = realloc(bs->pool, newsize * sizeof(newpool[0]));
			if (!newpool)
			{
				free(bs);
				return NULL;
			}
			bs->size = newsize;
			bs->pool = newpool;
		}
		bs->pool[bs->count++] = t;
	}

	return bs;
}

static inline int64_t iabs(int64_t a)
{
	return a > 0 ? a : -a;
}

static int64_t manhattan(const struct bot *a, const struct bot *b)
{
	return iabs(a->x - b->x) + iabs(a->y - b->y) + iabs(a->z - b->z);
}

static size_t part1(const struct botset *bs)
{
	size_t count = 0;
	struct bot *b = bs->pool + bs->strongest;
	for (size_t i = 0; i < bs->count; i++)
	{
		if (manhattan(b, bs->pool+i) <= b->r)
		{
			count++;
		}
	}
	return count;
}

struct box
{
	int64_t x, y, z;	/* position of the upper left corner */
	int64_t side;		/* length of a side */
	int64_t distance;	/* distance from the origin */
	size_t  botcount;	/* number of bots who influence the box */
};

static inline int64_t distance(int64_t x, int64_t low, int64_t high)
{
	if (x < low) return low - x;
	if (x > high) return x - high;
	return 0;
}

static int box_cmp(const struct box *a, const struct box *b)
{
	if (a->botcount != b->botcount)
	{
		return b->botcount - a->botcount;
	}
	if (a->distance != b->distance)
	{
		return a->distance - b->distance;
	}
	return a->side - b->side;
}

static void box_find_botcount(struct box *box, const struct botset *bs)
{
	size_t count = 0;
	for (struct bot *b = bs->pool; b != bs->pool + bs->count; b++)
	{
		int64_t d = 0;
		d += distance(b->x, box->x, box->x + box->side - 1);
		d += distance(b->y, box->y, box->y + box->side - 1);
		d += distance(b->z, box->z, box->z + box->side - 1);
		if (d <= b->r)
		{
			count++;
		}
	}
	box->botcount = count;
}

struct heap
{
	struct box *data;
	size_t count;
	size_t size;
};

static void heap_destroy(struct heap *h)
{
	free(h->data);
}

static void bubble_up(struct heap *h, size_t pos)
{
	while (pos > 0)
	{
		size_t par = (pos - 1) / 2;
		if (box_cmp(&h->data[par], &h->data[pos]) < 0)
		{
			break;
		}

		struct box t = h->data[par];
		h->data[par] = h->data[pos];
		h->data[pos] = t;

		pos = par;
	}
}

static void heap_push(struct heap *h, struct box b)
{
	if (h->count == h->size)
	{
		size_t nsize = h->size ? h->size * 2 : 64;
		struct box *ndata = realloc(h->data, nsize * sizeof(*ndata));
		if (!ndata)
		{
			abort();
		}
		h->size = nsize;
		h->data = ndata;
	}
	h->data[h->count] = b;
	bubble_up(h, h->count);
	h->count++;
}

static void bubble_down(struct heap *h, size_t pos)
{
	for (;;)
	{
		size_t min = pos;
		for (size_t i = pos * 2 + 1; i <= pos * 2 + 2; i++)
		{
			if (i < h->count && box_cmp(&h->data[i], &h->data[min]) < 0)
			{
				min = i;
			}
		}

		if (min == pos)
		{
			break;
		}

		struct box t = h->data[min];
		h->data[min] = h->data[pos];
		h->data[pos] = t;

		pos = min;
	}
}

static struct box heap_pop(struct heap *h)
{
	assert(h->count);
	struct box b = h->data[0];
	h->data[0] = h->data[--h->count];
	bubble_down(h, 0);
	return b;
}

static size_t part2(struct botset *bs)
{
	struct bot min = { INT64_MAX, INT64_MAX, INT64_MAX, 0 };
	struct bot max = { INT64_MIN, INT64_MIN, INT64_MIN, 0 };
	for (struct bot *b = bs->pool; b < bs->pool + bs->count; b++)
	{
		if (min.x > b->x - b->r) min.x = b->x - b->r;
		if (min.y > b->y - b->r) min.y = b->y - b->r;
		if (min.z > b->z - b->r) min.z = b->z - b->r;
		if (max.x < b->x + b->r) max.x = b->x + b->r;
		if (max.y < b->y + b->r) max.y = b->y + b->r;
		if (max.z < b->z + b->r) max.z = b->z + b->r;
	}
	struct box b = {
		.x = min.x,
		.y = min.y,
		.z = min.z,
		.distance = iabs(b.x) + iabs(b.y) + iabs(b.z),
		.side = 1
	};
	while (min.x + b.side < max.x || min.y + b.side < max.y || min.z + b.side < max.z)
	{
		b.side *= 2;
	}
	box_find_botcount(&b, bs);

	struct heap h = {};
	heap_push(&h, b);
	while (h.count)
	{
		b = heap_pop(&h);
		if (b.side == 1)
		{
			break;
		}
		struct box nb = b;
		nb.side = b.side / 2;
		for (nb.x = b.x; nb.x < b.x+b.side; nb.x += nb.side)
		{
			for (nb.y = b.y; nb.y < b.y + b.side; nb.y += nb.side)
			{
				for (nb.z = b.z; nb.z < b.z+b.side; nb.z += nb.side)
				{
					nb.distance = iabs(nb.x) + iabs(nb.y) + iabs(nb.z);
					box_find_botcount(&nb, bs);
					heap_push(&h, nb);
				}
			}
		}
	}
	heap_destroy(&h);
	return b.distance;
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

	struct botset *bs = botset_load(input);
	fclose(input);
	if (!bs)
	{
		fprintf(stderr, "Cannot parse the data\n");
		return 1;
	}

	printf("Part1: %zu\n", part1(bs));
	printf("Part2: %zu\n", part2(bs));

	botset_free(bs);
	return 0;
}
