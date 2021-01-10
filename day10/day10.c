#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

struct Vec
{
	int x;
	int y;
};

struct Light
{
	struct Vec pos;
	struct Vec vel;
};

static struct Vec light_pos(struct Light *l, int time)
{
	return (struct Vec){
		l->pos.x + l->vel.x * time,
		l->pos.y + l->vel.y * time,
	};
}

struct Range
{
	int x, y;
	int width, height;
};

static struct Range find_range(struct Light *lights, size_t count, int time)
{
	int xmin, ymin, xmax, ymax;
	xmin = ymin = INT_MAX;
	xmax = ymax = INT_MIN;
	for(struct Light *l = lights + count - 1;
	    l >= lights;
	    l--)
	{
		struct Vec p = light_pos(l, time);
		if (xmin > p.x) xmin = p.x;
		if (xmax < p.x) xmax = p.x;
		if (ymin > p.y) ymin = p.y;
		if (ymax < p.y) ymax = p.y;
	}
	return (struct Range){
		xmin, ymin,
		xmax-xmin+1, ymax-ymin+1
	};
}

static size_t span_area(struct Light *lights, size_t count, int time)
{
	struct Range r = find_range(lights, count, time);
	return (size_t)r.width * r.height;
}

static int absolute(int a)
{
	return a < 0 ? -a : a;
}

static int find_local_minimum(struct Light *lights, size_t count, int a, int b)
{
	/* trisection */
	while (absolute(a-b) > 2)
	{
		int u = a + (b-a) / 3;
		int v = b - (b-a) / 3;
		if (span_area(lights, count, u) < span_area(lights, count, v))
		{
			b = v;
		}
		else
		{
			a = u;
		}
	}

	/* the solution is in the interval [a..b] */
	size_t minarea = span_area(lights, count, b);
	int time = b;
	for (int i = a; i < b; i++)
	{
		size_t area = span_area(lights, count, i);
		if (minarea > area)
		{
			minarea = area;
			time = i;
		}
	}
	return time;
}

static void render(struct Light *lights, size_t count, int time)
{
	struct Vec *pos = malloc(count * sizeof(pos[0]));
	if (!pos)
	{
		return;
	}
	for (size_t i = 0; i < count; i++)
	{
		pos[i] = light_pos(lights+i, time);
	}

	struct Range r = find_range(lights, count, time);
	for (int y = r.y; y < r.y + r.height; y++)
	{
		for (int x = r.x; x < r.x+r.width; x++)
		{
			char c = ' ';
			for (size_t i = 0; i < count; i++)
			{
				if (pos[i].x == x && pos[i].y == y)
				{
					c = '#';
					break;
				}
			}
			putc(c, stdout);
		}
		putc('\n', stdout);
	}
	free(pos);
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

	struct Light *lights = NULL;
	size_t count = 0, size = 0;
	struct Vec pos, vel;
	while (fscanf(input, " position=<%d, %d> velocity=<%d, %d>",
			   &pos.x, &pos.y, &vel.x, &vel.y) == 4)
	{
		if (count == size)
		{
			size_t newsize = size ? size * 2 : 2;
			struct Light *newlights = realloc(
				lights,	sizeof(newlights[0]) * newsize);
			if (newlights == NULL)
			{
				break;
			}
			lights = newlights;
			size = newsize;
		}
		lights[count].pos = pos;
		lights[count].vel = vel;
		count++;
	}
	fclose(input);

	int time = find_local_minimum(lights, count, 0, 100000);
	printf("Part1: (see below)\n");
	printf("Part2: %d\n", time);
	render(lights, count, time);
	free(lights);
	return 0;
}
