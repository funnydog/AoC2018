#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSIZE 26

struct Node
{
	int name;
	int degree;
	int adj[MAXSIZE];
	size_t adjcount;
};

static void node_add_adj(struct Node *n, int adj)
{
	assert(n->adjcount < MAXSIZE);
	n->adj[n->adjcount++] = adj;
}

struct Graph
{
	struct Node node[MAXSIZE];
	int index[MAXSIZE];
	size_t count;
};

static struct Node *graph_get_node(struct Graph *g, size_t name)
{
	if (name >= MAXSIZE)
	{
		return NULL;
	}
	if (g->index[name] == -1)
	{
		g->index[name] = g->count;
		g->node[g->count].name = name;
		g->count++;
	}
	return g->node + g->index[name];
}

static int graph_load(FILE *input, struct Graph *g)
{
	memset(g, 0, sizeof(*g));
	memset(g->index, -1, sizeof(g->index));
	char before, after;
	while (fscanf(input,
		      " Step %c must be finished before step %c can begin.",
		      &before, &after) == 2)
	{
		/* add before and after to the graph */
		before -= 'A';
		after -= 'A';
		struct Node *n = graph_get_node(g, after);
		n->degree++;

		n = graph_get_node(g, before);
		node_add_adj(n, after);
	}
	return 0;
}

struct pair
{
	int priority;
	int value;
};

static int pair_cmp(const struct pair *a, const struct pair *b)
{
	return a->priority - b->priority;
}

struct heap
{
	struct pair data[MAXSIZE];
	size_t count;
};

static void bubble_up(struct heap *h, size_t pos)
{
	while (pos > 0)
	{
		size_t parent = (pos - 1) / 2;
		if (pair_cmp(h->data + parent, h->data + pos) < 0)
		{
			break;
		}
		/* swap */
		struct pair t = h->data[parent];
		h->data[parent] = h->data[pos];
		h->data[pos] = t;

		pos = parent;
	}
}

static void heap_push(struct heap *h, struct pair p)
{
	assert(h->count < MAXSIZE);
	h->data[h->count] = p;
	bubble_up(h, h->count);
	h->count++;
}

static void bubble_down(struct heap *h, size_t pos)
{
	for (;;)
	{
		size_t min = pos;
		for (size_t i = pos*2+1; i <= pos*2+2; i++)
		{
			if (i < h->count && pair_cmp(h->data+i, h->data+min) < 0)
			{
				min = i;
			}
		}
		if (min == pos)
		{
			break;
		}
		/* swap */
		struct pair t = h->data[min];
		h->data[min] = h->data[pos];
		h->data[pos] = t;

		pos = min;
	}
}

static struct pair heap_pop(struct heap *h)
{
	assert(h->count);
	struct pair p = h->data[0];
	h->data[0] = h->data[--h->count];
	bubble_down(h, 0);
	return p;
}

static void graph_schedule(struct Graph *g, size_t workers, char *buffer, int *time)
{
	*time = 0;
	struct heap s = {0};
	struct heap jobs = {0};
	int degree[MAXSIZE];
	for (size_t i = 0; i < g->count; i++)
	{
		degree[i] = g->node[i].degree;
		if (!degree[i])
		{
			struct pair p = {g->node[i].name, g->node[i].name};
			heap_push(&s, p);
		}
	}
	for (;;)
	{
		while (s.count && jobs.count < workers)
		{
			struct pair p = heap_pop(&s);
			p.priority = *time + 60 + p.value + 1;
			heap_push(&jobs, p);
		}

		if (!jobs.count)
		{
			break;
		}

		struct pair p = heap_pop(&jobs);
		*time = p.priority;
		*buffer++ = 'A' + p.value;
		struct Node *n = graph_get_node(g, p.value);
		for (size_t i = 0; i < n->adjcount; i++)
		{
			struct Node *m = graph_get_node(g, n->adj[i]);
			degree[g->index[n->adj[i]]]--;
			if (degree[g->index[n->adj[i]]] == 0)
			{
				p.priority = m->name;
				p.value = m->name;
				heap_push(&s, p);
			}
		}
	}
	*buffer = 0;
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
	struct Graph g;
	graph_load(input, &g);
	fclose(input);

	int time;
	char buffer[27];
	graph_schedule(&g, 1, buffer, &time);
	printf("Part1: %s\n", buffer);
	graph_schedule(&g, 5, buffer, &time);
	printf("Part2: %d\n", time);
	return 0;
}
