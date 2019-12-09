#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum guard_state
{
	BEGINS_SHIFT = 0,
	FALLS_ASLEEP = 1,
	WAKES_UP = 2,
};

struct event
{
	int year;
	int month;
	int day;

	int hour;
	int minute;

	int id;
	enum guard_state state;
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
	int low = 0;
	int high = a->count-1;
	while (low < high) {
		int mid = low + (high - low) / 2;
		if (guard_id > a->pool[mid].id) {
			low = mid + 1;
		} else {
			high = mid;
		}
	}
	if (pos)
		*pos = (high >= 0) ? high : 0;

	if (a->count && guard_id == a->pool[high].id)
		return 1;

	return 0;
}

static int add_guard(struct guard_array *a, int guard_id)
{
	size_t pos;
	if (find_guard(a, guard_id, &pos))
		return -1;

	if (a->count == a->size) {
		size_t newsize = a->size ? a->size * 2 : 2;
		struct guard *newpool = realloc(a->pool, newsize * sizeof(*newpool));
		if (newpool == NULL)
			return -1;

		a->pool = newpool;
		a->size = newsize;
	}

	memmove(a->pool + pos + 1, a->pool + pos, sizeof(a->pool[0]) * (a->count-pos));
	memset(&a->pool[pos], 0, sizeof(a->pool[0]));
	a->pool[pos].id = guard_id;
	a->count++;

	return pos;
}

static int read_event(FILE *input, struct event *t)
{
	int r = fscanf(input, " [%d-%d-%d %d:%d]",
		       &t->year, &t->month, &t->day, &t->hour, &t->minute);
	if (r != 5) {
		return EOF;
	}

	char *line = NULL;
	size_t linesize = 0;
	if (getline(&line, &linesize, input) == -1) {
		free(line);
		return EOF;
	}
	if (sscanf(line, " Guard #%d begins shift", &t->id) == 1) {
		t->state = BEGINS_SHIFT;
	} else if (strcmp(line, " wakes up\n") == 0) {
		t->id = 0;
		t->state = WAKES_UP;
	} else if (strcmp(line, " falls asleep\n") == 0) {
		t->id = 0;
		t->state = FALLS_ASLEEP;
	} else {
		free(line);
		return EOF;
	}

	free(line);
	return 1;
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
	if (argc < 2) {
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		return -1;
	}

	FILE *input = fopen(argv[1], "rb");
	if (!input) {
		fprintf(stderr, "File %s not found\n", argv[1]);
		return -1;
	}

	/* read the events and sort them */
	struct event e;
	struct event *events = NULL;
	size_t count = 0;
	size_t size = 0;
	while (read_event(input, &e) != EOF) {
		if (count == size) {
			size_t newsize = size ? size * 2 : 8;
			struct event *newevents = realloc(events, newsize * sizeof(*newevents));
			if (newevents == NULL) {
				return -1;
			}
			size = newsize;
			events = newevents;
		}
		events[count] = e;
		count++;
	}
	qsort(events, count, sizeof(*events), event_cmp);

	/* fill the asleep schedule for each guard */
	struct guard_array ga = {0};
	int id = -1;
	int min = 0;
	int last_state = -1;
	for (struct event *e = events; e < events + count; e++) {
		size_t pos;
		switch (e->state) {
		case BEGINS_SHIFT:
			if (last_state == FALLS_ASLEEP) {
				if (find_guard(&ga, id, &pos)) {
					for (; min < 60; min++)
						ga.pool[pos].minutes[min]++;
				}
			}

			id = e->id;
			add_guard(&ga, id);
			break;

		case FALLS_ASLEEP:
			min = e->minute;
			break;

		case WAKES_UP:
			if (find_guard(&ga, id, &pos)) {
				for (; min < e->minute; min++)
					ga.pool[pos].minutes[min]++;
			}
			break;
		}
		last_state = e->state;
	}

	/* compute the tot_asleep, max_asleep, max_asleep_minute */
	for (struct guard *g = ga.pool; g < ga.pool + ga.count; g++) {
		g->tot_asleep = 0;
		g->max_asleep = -1;
		g->max_asleep_minute = -1;
		for (int i = 0; i < 60; i++) {
			g->tot_asleep += g->minutes[i];
			if (g->max_asleep < g->minutes[i]) {
				g->max_asleep = g->minutes[i];
				g->max_asleep_minute = i;
			}
		}
	}

	/* sort by tot_asleep */
	qsort(ga.pool, ga.count, sizeof(ga.pool[0]), guard_totasleep_cmp);
	printf("Strategy 1: Guard ID: %d, Total Asleep: %d, Minute %d, Product: %d\n",
	       ga.pool[0].id, ga.pool[0].tot_asleep, ga.pool[0].max_asleep_minute,
	       ga.pool[0].id * ga.pool[0].max_asleep_minute);

	/* sort by max_asleep */
	qsort(ga.pool, ga.count, sizeof(ga.pool[0]), guard_maxasleep_cmp);
	printf("Strategy 2: Guard ID: %d, Freq. Asleep: %d, Minute %d, Product: %d\n",
	       ga.pool[0].id, ga.pool[0].max_asleep, ga.pool[0].max_asleep_minute,
	       ga.pool[0].id * ga.pool[0].max_asleep_minute);

	free(ga.pool);
	free(events);
	fclose(input);
	return 0;
}
