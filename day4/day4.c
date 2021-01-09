#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct event
{
	int year;
	int month;
	int day;

	int hour;
	int minute;

	int id;

	enum { BEGINS_SHIFT, FALLS_ASLEEP, WAKES_UP } type;
};

struct guard
{
	int id;
	int minutes[60];

	int tot_asleep;
	int max_asleep;
	int max_asleep_minute;
};

struct guard_array
{
	struct guard *pool;
	size_t count;
	size_t size;
};

static int find_guard(struct guard_array *a, int guard_id, size_t *pos)
{
	if (a->count == 0)
	{
		if (pos)
		{
			*pos = 0;
		}
		return 0;
	}

	size_t low = 0;
	size_t high = a->count;
	while (low < high)
	{
		int mid = low + (high - low) / 2;
		if (a->pool[mid].id < guard_id)
		{
			low = mid + 1;
		}
		else
		{
			high = mid;
		}
	}

	if (pos)
	{
		*pos = low;
	}

	return guard_id == a->pool[low].id;
}

static int add_guard(struct guard_array *a, int guard_id)
{
	size_t pos;
	if (find_guard(a, guard_id, &pos))
	{
		return -1;
	}

	if (a->count == a->size)
	{
		size_t newsize = a->size ? a->size * 2 : 2;
		struct guard *newpool = realloc(a->pool, newsize * sizeof(*newpool));
		if (newpool == NULL)
		{
			return -1;
		}
		a->pool = newpool;
		a->size = newsize;
	}

	memmove(a->pool + pos + 1, a->pool + pos, sizeof(a->pool[0]) * (a->count-pos));
	memset(&a->pool[pos], 0, sizeof(a->pool[0]));
	a->pool[pos].id = guard_id;
	a->count++;
	return pos;
}

static int parse_event(const char *line, struct event *t)
{
	if (sscanf(line,
		   "[%d-%d-%d %d:%d] Guard #%d begins shift\n",
		   &t->year, &t->month, &t->day, &t->hour, &t->minute, &t->id) == 6)
	{
		t->type = BEGINS_SHIFT;
	}
	else if (strcmp(line+19, "wakes up\n") == 0)
	{
		t->id = -1;
		t->type = WAKES_UP;
	}
	else if (strcmp(line+19, "falls asleep\n") == 0)
	{
		t->id = -1;
		t->type = FALLS_ASLEEP;
	}
	else
	{
		return -1;
	}
	return 0;
}

static int event_cmp(const void *d1, const void *d2)
{
	const struct event *a = d1;
	const struct event *b = d2;
	if (a->year != b->year)
		return a->year - b->year;
	if (a->month != b->month)
		return a->month - b->month;
	if (a->day != b->day)
		return a->day - b->day;
	if (a->hour != b->hour)
		return a->hour - b->hour;
	return a->minute - b->minute;
}

static int parse_guards(FILE *input, struct guard_array *ga)
{
	struct event *events = NULL;
	size_t count = 0;
	size_t size = 0;
	char *line = NULL;
	size_t sline = 0;
	while (getline(&line, &sline, input) != -1)
	{
		struct event e;
		if (parse_event(line, &e) < 0)
		{
			break;
		}
		else if (count == size)
		{
			size_t newsize = size ? size * 2 : 8;
			struct event *newevents = realloc(events, newsize * sizeof(*newevents));
			if (!newevents)
			{
				break;
			}
			size = newsize;
			events = newevents;
		}
		events[count] = e;
		count++;
	}
	free(line);
	qsort(events, count, sizeof(events[0]), event_cmp);

	/* fill the asleep schedule for each guard */
	memset(ga, 0, sizeof(*ga));
	int guard = -1;
	struct event *last = NULL;
	for (struct event *e = events; e < events + count; e++)
	{
		size_t pos;
		switch (e->type) {
		case BEGINS_SHIFT:
			guard = e->id;
			add_guard(ga, guard);
			last = NULL;
			break;

		case WAKES_UP:
			if (last)
			{
				if (find_guard(ga, guard, &pos))
				{
					for (int i = last->minute; i < e->minute; i++)
					{
						ga->pool[pos].minutes[i]++;
					}
				}
			}
			last = NULL;
			break;

		case FALLS_ASLEEP:
			last = e;
			break;
		}
	}
	free(events);

	/* compute the tot_asleep, max_asleep, max_asleep_minute */
	for (struct guard *g = ga->pool; g < ga->pool + ga->count; g++)
	{
		g->tot_asleep = 0;
		g->max_asleep = 0;
		g->max_asleep_minute = -1;
		for (int i = 0; i < 60; i++)
		{
			g->tot_asleep += g->minutes[i];
			if (g->max_asleep < g->minutes[i])
			{
				g->max_asleep = g->minutes[i];
				g->max_asleep_minute = i;
			}
		}
	}
	return 0;
}

static int guard_totasleep_cmp(const void *d1, const void *d2)
{
	const struct guard *a = d1;
	const struct guard *b = d2;
	return b->tot_asleep - a->tot_asleep;
}

static int guard_maxasleep_cmp(const void *d1, const void *d2)
{
	const struct guard *a = d1;
	const struct guard *b = d2;
	return b->max_asleep - a->max_asleep;
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

	/* parse the guard array from the file */
	struct guard_array ga;
	if (parse_guards(input, &ga) < 0)
	{
		fclose(input);
		fprintf(stderr, "Cannot parse the data\n");
		return 1;
	}
	fclose(input);

	/* sort by tot_asleep */
	qsort(ga.pool, ga.count, sizeof(ga.pool[0]), guard_totasleep_cmp);
	printf("Part1: %d\n", ga.pool[0].id * ga.pool[0].max_asleep_minute);

	/* sort by max_asleep */
	qsort(ga.pool, ga.count, sizeof(ga.pool[0]), guard_maxasleep_cmp);
	printf("Part2: %d\n", ga.pool[0].id * ga.pool[0].max_asleep_minute);

	free(ga.pool);
	return 0;
}
