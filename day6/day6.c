#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

struct location
{
	int id;
	int x;
	int y;
	int skip;
	int area;
};

struct region
{
	int x;
	int y;
	int w;
	int h;

	int *map;
};

static int distance(int x, int y)
{
	return (x > 0 ? x : -x) + (y > 0 ? y : -y);
}

static int region_init(struct region *out, struct location *l, size_t size)
{
	int x_min, y_min;
	int x_max, y_max;

	x_min = y_min = INT_MAX;
	x_max = y_max = INT_MIN;
	while (size-->0)
	{
		if (x_min > l->x) x_min = l->x;
		if (x_max < l->x) x_max = l->x;
		if (y_min > l->y) y_min = l->y;
		if (y_max < l->y) y_max = l->y;
		l++;
	}

	out->x = x_min;
	out->w = x_max - x_min + 1;
	out->y = y_min;
	out->h = y_max - y_min + 1;

	out->map = malloc(out->w * out->h * sizeof(out->map[0]));
	if (!out->map)
	{
		return -1;
	}

	return 0;
}

static int region_find_area(struct region *r, const struct location *l)
{
	int area = 0;
	for (int y = 0; y < r->h; y++)
	{
		for (int x = 0; x < r->w; x++)
		{
			int label = r->map[y * r->w + x];
			if (label == l->id)
			{
				if (x == 0 || x == r->w - 1 ||
				    y == 0 || y == r->h - 1)
					return 0;
				area++;
			}
		}
	}
	return area;
}

static int region_max_area(struct region *r, const struct location *locs, size_t count)
{
	for (int y = 0; y < r->h; y++)
	{
		for (int x = 0; x < r->w; x++)
		{
			int minlabel = -1;
			int mind = INT_MAX;
			for (const struct location *l = locs + count - 1;
			     l >= locs;
			     l--)
			{
				int d = distance(x + r->x - l->x, y + r->y - l->y);
				if (d == mind)
				{
					minlabel = -1;
				}
				else if (d < mind)
				{
					mind = d;
					minlabel = l->id;
				}
			}
			r->map[y * r->w + x] = minlabel;
		}
	}

	int maxarea = INT_MIN;
	for (const struct location *l = locs + count - 1;
	     l >= locs;
	     l--)
	{
		int area = region_find_area(r, l);
		if (maxarea < area)
		{
			maxarea = area;
		}
	}
	return maxarea;
}

static int region_area_lt(struct region *r, const struct location *locs, size_t count, int limit)
{
	for (int y = 0; y < r->h; y++)
	{
		for (int x = 0; x < r->w; x++)
		{
			int sum = 0;
			for (const struct location *l = locs + count - 1;
			     l >= locs;
			     l--)
			{
				sum += distance(x + r->x - l->x, y + r->y - l->y);
			}
			r->map[y * r->w + x] = sum;
		}
	}

	int area = 0;
	for (int y = 0; y < r->h; y++)
	{
		for (int x = 0; x < r->w; x++)
		{
			if (r->map[y * r->w + x] < limit)
			{
				area++;
			}
		}
	}
	return area;
}

static void region_destroy(struct region *r)
{
	free(r->map);
	r->map = NULL;
	r->x = r->y = r->w = r->h = 0;
}

struct location *load_locations(FILE *input, size_t *count)
{
	size_t size = 0;
	struct location l, *pool = NULL;
	*count = 0;
	while (fscanf(input, " %d, %d", &l.x, &l.y) == 2)
	{
		l.id = *count;
		l.skip = l.area = 0;
		if (*count == size)
		{
			size_t newsize = size ? size * 2 : 2;
			struct location *newpool = realloc(
				pool, newsize * sizeof(newpool[0]));
			if (newpool == NULL)
			{
				break;
			}
			pool = newpool;
			size = newsize;
		}
		pool[*count] = l;
		(*count)++;
	}
	return pool;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		return 1;
	}

	FILE *input = fopen(argv[1], "rb");
	if (!input)
	{
		fprintf(stderr, "Cannot open %s\n", argv[1]);
		return 1;
	}

	size_t count;
	struct location *locs = load_locations(input, &count);
	fclose(input);

	struct region r;
	if (region_init(&r, locs, count) < 0)
	{
		fprintf(stderr, "Cannot initialize the region\n");
		free(locs);
		return 1;
	}
	printf("Part1: %d\n", region_max_area(&r, locs, count));
	printf("Part2: %d\n", region_area_lt(&r, locs, count, 10000));
	region_destroy(&r);
	free(locs);

	return 0;
}
