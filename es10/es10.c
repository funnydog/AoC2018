#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

struct light
{
	long x, y;
	long vx, vy;
};

static void light_forward(struct light *l)
{
	l->x += l->vx;
	l->y += l->vy;
}

static void light_backwards(struct light *l)
{
	l->x -= l->vx;
	l->y -= l->vy;
}

static void foreach(struct light *lights, size_t count, void (*fn)(struct light*))
{
	for (size_t i = 0; i < count; i++) {
		fn(lights + i);
	}
}

struct extents
{
	long xmin, ymin;
	long xmax, ymax;
};

static long spanning_area(struct light *lights, size_t count, struct extents *m)
{
	struct extents e = {
		.xmin = LONG_MAX,
		.ymin = LONG_MAX,
		.xmax = LONG_MIN,
		.ymax = LONG_MIN,
	};
	for (size_t i = 0 ; i < count; i++) {
		if (lights[i].x < e.xmin) e.xmin = lights[i].x;
		if (lights[i].x > e.xmax) e.xmax = lights[i].x;
		if (lights[i].y < e.ymin) e.ymin = lights[i].y;
		if (lights[i].y > e.ymax) e.ymax = lights[i].y;
	}
	if (m)
		*m = e;

	return (e.xmax - e.xmin + 1) * (e.ymax - e.ymin + 1);
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

	struct light *lights = NULL;
	size_t lcount = 0, lsize = 0;
	long x, y, vx, vy;
	while (fscanf(input, " position=<%ld, %ld> velocity=<%ld, %ld>",
			   &x, &y, &vx, &vy) == 4) {
		if (lcount == lsize) {
			size_t newsize = lsize ? lsize * 2 : 2;
			struct light *newlights = realloc(
				lights,	sizeof(newlights[0]) * newsize);
			if (newlights == NULL)
				break;
			lights = newlights;
			lsize = newsize;
		}
		lights[lcount].x = x;
		lights[lcount].y = y;
		lights[lcount].vx = vx;
		lights[lcount].vy = vy;
		lcount++;
	}
	fclose(input);

	printf("Read %zu light points, reserved %zu\n", lcount, lsize);
	long minarea = LONG_MAX;
	long minseconds;
	long seconds;
	for (seconds = 0; ; seconds++) {
		long area = spanning_area(lights, lcount, NULL);
		if (minarea < area) {
			break;
		} else if (minarea > area) {
			minseconds = seconds;
			minarea = area;
		}
		foreach(lights, lcount, light_forward);
	}
	printf("Minimum reached after %ld seconds, area %ld\n",
	       minseconds, minarea);

	while (seconds > minseconds) {
		foreach(lights, lcount, light_backwards);
		seconds--;
	}

	struct extents e;
	spanning_area(lights, lcount, &e);
	printf("Map size %ldx%ld starting at (%ld,%ld)\n",
	       e.xmax - e.xmin + 1, e.ymax - e.ymin + 1, e.xmin, e.ymin);

	for (long y = e.ymin; y <= e.ymax; y++) {
		for (long x = e.xmin; x <= e.xmax; x++) {
			size_t i;
			for (i = 0; i < lcount; i++) {
				if (lights[i].x == x && lights[i].y == y) {
					fputc('#', stdout);
					break;
				}
			}
			if (i == lcount) {
				fputc(' ', stdout);
			}
		}
		fputc('\n', stdout);
	}

	free(lights);
	return 0;
}
