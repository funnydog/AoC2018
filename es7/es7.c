#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STEPSIZE ('Z' - 'A' + 1)

struct taskpool
{
	struct task
	{
		int name;
		int index;
		int assigned;
		int to_complete;
		int depends[STEPSIZE];
	} tasks[STEPSIZE];
};

struct taskpool *taskpool_new(void)
{
	return calloc(1, sizeof(struct taskpool));
}

void taskpool_free(struct taskpool *tp)
{
	free(tp);
}

void taskpool_add_rule(struct taskpool *tp, int task, int requisite)
{
	task -= 'A';
	requisite -= 'A';

	tp->tasks[task].name = task + 'A';
	tp->tasks[requisite].name = requisite + 'A';

	tp->tasks[task].to_complete = 1;
	tp->tasks[requisite].to_complete = 1;

	tp->tasks[task].depends[requisite] = 1;
}

struct task *taskpool_find_available(struct taskpool *tp)
{
	struct task *t = NULL;
	for (int i = 0; i < STEPSIZE; i++) {
		if (tp->tasks[i].assigned || !tp->tasks[i].to_complete)
			continue;

		int j;
		for (j = 0; j < STEPSIZE; j++) {
			if (tp->tasks[i].depends[j] && tp->tasks[j].to_complete)
				break;
		}
		if (j == STEPSIZE &&
		    (t == NULL || tp->tasks[i].name < t->name))
			t = tp->tasks + i;
	}
	return t;
}

struct workpool
{
	struct worker
	{
		int end;
		struct task *task;
	} *pool;
	size_t count;
	size_t size;

	int done[STEPSIZE];
	size_t donecount;
};

struct workpool *workpool_new(size_t size)
{
	struct workpool *wp = calloc(1, sizeof(*wp) + size * sizeof(wp->pool[0]));
	if (wp) {
		wp->pool = (void *)((char *)wp + sizeof(*wp));
		wp->count = 0;
		wp->size = size;
	}
	return wp;
}

void workpool_free(struct workpool *wp)
{
	free(wp);
}

struct worker *workpool_get(struct workpool *wp)
{
	if (wp->count == wp->size)
		return NULL;

	for (size_t i = 0; i < wp->size; i++) {
		if (wp->pool[i].task == NULL) {
			wp->count++;
			return wp->pool + i;
		}
	}

	return NULL;
}

int workpool_assign_task(struct workpool *wp, struct task *t, int now)
{
	struct worker *w = workpool_get(wp);
	if (w) {
		w->task = t;
		w->end = now + t->name - 'A' + 61;
		t->assigned = 1;
		return 1;
	}
	return 0;
}

int workpool_process(struct workpool *wp, int now)
{
	int complete = 0;
	for (size_t i = 0; i < wp->size; i++) {
		if (wp->pool[i].task && wp->pool[i].end == now) {
			wp->done[wp->donecount]= wp->pool[i].task->name;
			wp->donecount++;
			wp->pool[i].task->to_complete = 0;
			wp->pool[i].task = NULL;
			wp->count--;
			complete++;
		}
	}
	return complete;
}

size_t workpool_count(struct workpool *wp)
{
	return wp->count;
}

void workpool_print(struct workpool *wp, int now)
{
	printf("%4d", now);
	for (size_t i = 0; i < wp->size; i++) {
		printf(" %c ", wp->pool[i].task ? wp->pool[i].task->name : '.');
	}
	for (size_t i = 0; i < wp->donecount; i++) {
		printf("%c", wp->done[i]);
	}
	printf("\n");
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "Usage: %s <filename> <numofworkers>\n", argv[0]);
		return -1;
	}

	char *end;
	size_t wsize = strtoul(argv[2], &end, 10);
	if (end == argv[2] || *end != 0) {
		fprintf(stderr, "Cannot parse the number of workers: %s\n", argv[2]);
		return -1;
	}

	FILE *input = fopen(argv[1], "rb");
	if (!input) {
		fprintf(stderr, "Cannot open %s for reading\n", argv[1]);
		return -1;
	}

	struct taskpool *tp = taskpool_new();
	if (!tp) {
		fclose(input);
		return -1;
	}

	char task, requisite;
	while (fscanf(input, " Step %c must be finished before step %c can begin.",
		      &requisite, &task) == 2) {
		taskpool_add_rule(tp, task, requisite);
	}
	fclose(input);

	struct workpool *wp = workpool_new(wsize);
	if (!wp) {
		taskpool_free(tp);
		return -1;
	}

	int now = 0;
	for (;;) {
		struct task *t;
		while ((t = taskpool_find_available(tp)) != NULL) {
			if (workpool_assign_task(wp, t, now) == 0)
				break;
		}

		workpool_print(wp, now);

		if (workpool_count(wp) == 0)
			break;

		now = INT_MAX;
		for (size_t i = 0; i < wp->size; i++) {
			if (wp->pool[i].task && wp->pool[i].end < now)
				now = wp->pool[i].end;
		}
		workpool_process(wp, now);
	}
	workpool_free(wp);
	taskpool_free(tp);

	printf("Seconds: %d\n", now);

	return 0;
}
