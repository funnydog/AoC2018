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

static int64_t distance(const struct bot *a, const struct bot *b)
{
	return iabs(a->x - b->x) + iabs(a->y - b->y) + iabs(a->z - b->z);
}

static size_t botset_inrange(struct botset *bs, size_t pos)
{
	size_t count = 0;
	struct bot *b = bs->pool+pos;
	for (size_t i = 0; i < bs->count; i++)
	{
		if (distance(b, bs->pool+i) <= b->r)
		{
			count++;
		}
	}
	return count;
}

struct box
{
	int64_t x, y, z;
	int64_t side;
	int64_t distance;
	size_t  botcount;
};

static inline int64_t range(int64_t x, int64_t low, int64_t high)
{
	if (x < low) return low - x;
	if (x > high) return x - high;
	return 0;
}

static size_t count_bots(struct box *box, struct botset *bs)
{
	size_t count = 0;
	for (size_t i = 0; i < bs->count; i++)
	{
		struct bot *b = bs->pool + i;
		int64_t d = 0;
		d += range(b->x, box->x, box->x + box->side - 1);
		d += range(b->y, box->y, box->y + box->side - 1);
		d += range(b->z, box->z, box->z + box->side - 1);
		if (d <= b->r)
		{
			count++;
		}
	}
	return count;
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

static int heap_lt(const struct box *a, const struct box *b)
{
	if (a->botcount != b->botcount)
	{
		return a->botcount > b->botcount;
	}
	if (a->distance != b->distance)
	{
		return a->distance < b->distance;
	}
	return a->side < b->side;
}

static void bubble_up(struct heap *h, size_t pos)
{
	while (pos > 0)
	{
		size_t par = (pos - 1) / 2;
		if (heap_lt(&h->data[par], &h->data[pos]))
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
			if (i < h->count && heap_lt(&h->data[i], &h->data[min]))
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

static size_t botset_search(struct botset *bs)
{
	struct bot min = { INT64_MAX, INT64_MAX, INT64_MAX, 0 };
	struct bot max = { INT64_MIN, INT64_MIN, INT64_MIN, 0 };
	for (size_t i = 0; i < bs->count; i++)
	{
		struct bot *b = bs->pool + i;
		if (min.x > b->x - b->r) min.x = b->x - b->r;
		if (min.y > b->y - b->r) min.y = b->y - b->r;
		if (min.z > b->z - b->r) min.z = b->z - b->r;
		if (max.x < b->x + b->r) max.x = b->x + b->r;
		if (max.y < b->y + b->r) max.y = b->y + b->r;
		if (max.z < b->z + b->r) max.z = b->z + b->r;
	}

	struct box b;
	b.x = min.x;
	b.y = min.y;
	b.z = min.z;
	b.distance = iabs(b.x) + iabs(b.y) + iabs(b.z);
	b.side = 1;
	while (min.x + b.side < max.x || min.y + b.side < max.y || min.z + b.side < max.z)
	{
		b.side *= 2;
	}
	b.botcount = count_bots(&b, bs);

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
		for (int i = 0; i < 2; i++, nb.x += nb.side)
		{
			nb.y = b.y;
			for (int j = 0; j < 2; j++, nb.y += nb.side)
			{
				nb.z = b.z;
				for (int k = 0; k < 2; k++, nb.z += nb.side)
				{
					nb.distance = iabs(nb.x) + iabs(nb.y) + iabs(nb.z);
					nb.botcount = count_bots(&nb, bs);
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
		return -1;
	}

	FILE *input = fopen(argv[1], "rb");
	if (!input)
	{
		fprintf(stderr, "Cannot open %s for reading\n", argv[1]);
		return -1;
	}

	struct botset *bs = botset_load(input);
	fclose(input);
	if (!bs)
	{
		fprintf(stderr, "Cannot load the nanobot list\n");
		return -1;
	}

	printf("part1: %zu\n", botset_inrange(bs, bs->strongest));
	printf("part2: %zu\n", botset_search(bs));

	botset_free(bs);
	return 0;
}
