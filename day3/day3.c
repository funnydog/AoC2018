#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct claim
{
	int id;			/* identifier */
	int x;			/* x position */
	int y;			/* y position */
	int w;			/* width */
	int h;			/* height */

	struct claim *next;
};

int fabric[1000][1000];

static struct claim *load(FILE *input)
{
	struct claim *head = NULL;
	struct claim **tail = &head;
	struct claim c = {0};
	while (fscanf(input, " #%d @ %d,%d: %dx%d", &c.id, &c.x, &c.y, &c.w, &c.h) == 5)
	{
		struct claim *n = malloc(sizeof(*n));
		if (!n)
		{
			break;
		}
		*n = c;
		*tail = n;
		tail = &n->next;
	}
	return head;
}

static size_t part1(const struct claim *c)
{
	size_t count = 0;
	memset(fabric, 0, sizeof(fabric));
	for (; c; c = c->next)
	{
		for (int y = c->y; y < c->y+c->h; y++)
		{
			for (int x = c->x; x < c->x + c->w; x++)
			{
				fabric[y][x]++;
				if (fabric[y][x] == 2)
				{
					count++;
				}
			}
		}
	}
	return count;
}

static int part2(struct claim *c)
{
	/* part1 is a requisite */
	for(; c; c = c->next)
	{
		int found = 1;
		for (int y = c->y; found && y < c->y+c->h; y++)
		{
			for (int x = c->x; found && x < c->x+c->w; x++)
			{
				if (fabric[y][x] > 1)
				{
					found = 0;
				}
			}
		}
		if (found)
		{
			return c->id;
		}
	}
	return -1;
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
		fprintf(stderr, "File %s not found\n", argv[1]);
		return 1;
	}

	struct claim *lst = load(input);
	fclose(input);
	if (!lst)
	{
		fprintf(stderr, "Cannot parse the data\n");
		return 1;
	}

	printf("Part1: %zu\n", part1(lst));
	printf("Part2: %d\n", part2(lst));
	while (lst)
	{
		struct claim *tmp = lst;
		lst = lst->next;
		free(tmp);
	}
	return 0;
}
