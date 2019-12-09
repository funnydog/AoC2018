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

static int manhattan_distance(int x0, int y0, int x1, int y1)
{
	return ((x1 > x0) ? (x1 - x0) : (x0 - x1)) +
		((y1 > y0) ? (y1 - y0) : (y0 - y1));
}

static int region_init(struct region *out, struct location *locs, size_t size)
{
	int x_min, y_min;
	int x_max, y_max;

	x_min = y_min = INT_MAX;
	x_max = y_max = INT_MIN;
	for (struct location *l = locs+size-1; l >= locs; l--) {
		if (x_min > l->x) x_min = l->x;
		if (x_max < l->x) x_max = l->x;
		if (y_min > l->y) y_min = l->y;
		if (y_max < l->y) y_max = l->y;
	}

	out->x = x_min;
	out->w = x_max - x_min + 1;
	out->y = y_min;
	out->h = y_max - y_min + 1;

	out->map = malloc(out->w * out->h * sizeof(out->map[0]));
	if (!out->map)
		return -1;

	return 0;
}

static void region_partition(struct region *r, struct location *locs, size_t count)
{
	for (int y = 0; y < r->h; y++) {
		for (int x = 0; x < r->w; x++) {
			int minlabel = -1;
			int mind = INT_MAX;
			for (struct location *l = locs + count - 1; l >= locs; l--) {
				int d = manhattan_distance(
					x + r->x, y + r->y,
					l->x, l->y);
				if (d == mind) {
					minlabel = -1;
				} else if (d < mind) {
					mind = d;
					minlabel = l->id;
				}
			}
			r->map[y * r->w + x] = minlabel;
		}
	}
}

static int region_find_area(struct region *r, struct location *l)
{
	int area = 0;
	for (int y = 0; y < r->h; y++) {
		for (int x = 0; x < r->w; x++) {
			int label = r->map[y * r->w + x];
			if (label == l->id) {
				if (x == 0 || x == r->w - 1 ||
				    y == 0 || y == r->h - 1)
					return 0;
				area++;
			}
		}
	}
	return area;
}

static int region_max_area(struct region *r, struct location *locs, size_t count)
{
	region_partition(r, locs, count);
	int maxarea = INT_MIN;
	for (struct location *l = locs + count - 1; l >= locs; l--) {
		int area = region_find_area(r, l);
		if (maxarea < area) {
			maxarea = area;
		}
	}
	return maxarea;
}

static void region_dist_sum(struct region *r, struct location *locs, size_t count)
{
	for (int y = 0; y < r->h; y++) {
		for (int x = 0; x < r->w; x++) {
			int sum = 0;
			for (struct location *l = locs + count - 1; l >= locs; l--) {
				sum += manhattan_distance(
					x + r->x, y + r->y,
					l->x, l->y);
			}
			r->map[y * r->w + x] = sum;
		}
	}
}

static int region_area_lt(struct region *r, int limit)
{
	int area = 0;
	for (int y = 0; y < r->h; y++) {
		for (int x = 0; x < r->w; x++) {
			if (r->map[y * r->w + x] < limit)
				area++;
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

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		return -1;
	}

	FILE *input = fopen(argv[1], "rb");
	if (!input) {
		fprintf(stderr, "Cannot open %s\n", argv[1]);
		return -1;
	}

	struct location l, *pool = NULL;
	size_t count = 0;
	size_t size = 0;

	while (fscanf(input, " %d, %d", &l.x, &l.y) == 2) {
		l.id = count;
		l.skip = l.area = 0;
		if (count == size) {
			size_t newsize = size ? size * 2 : 2;
			struct location *newpool = realloc(
				pool, newsize * sizeof(newpool[0]));
			if (newpool == NULL) {
				break;
			}
			pool = newpool;
			size = newsize;
		}
		pool[count] = l;
		count++;
	}
	fclose(input);

	struct region r;
	if (region_init(&r, pool, count) < 0) {
		fprintf(stderr, "Cannot initialize the region\n");
		free(pool);
		return -1;
	}
	printf("Largest area that isn't infinite: %d\n",
	       region_max_area(&r, pool, count));

	region_dist_sum(&r, pool, count);
	printf("Size of the region with distance sum < 32: %d\n",
	       region_area_lt(&r, 32));

	printf("Size of the region with distance sum < 10000: %d\n",
	       region_area_lt(&r, 10000));

	region_destroy(&r);
	free(pool);

	return 0;
}
