#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct point
{
	int v[4];

	/* for disjoint-set data structure */
	size_t parent;
	size_t rank;
};

struct sky
{
	struct point *p;
	size_t count;
	size_t size;
};

static void sky_free(struct sky *s)
{
	if (s) {
		free(s->p);
		free(s);
	}
}

static struct sky *sky_load(FILE *input)
{
	struct sky *s = calloc(1, sizeof(*s));
	if (!s)
		return NULL;

	struct point pt = {0};
	while (fscanf(input, " %d,%d,%d,%d", pt.v+0,pt.v+1,pt.v+2,pt.v+3) == 4) {
		if (s->count == s->size) {
			size_t nsize = s->size ? s->size * 2 : 2;
			struct point *np = realloc(s->p, nsize * sizeof(np[0]));
			if (!np) {
				sky_free(s);
				return NULL;
			}
			s->size = nsize;
			s->p = np;
		}
		pt.parent = s->count;
		pt.rank = 0;
		s->p[s->count++] = pt;
	}
	return s;
}

static int sky_djs_find(struct sky *s, size_t i)
{
	/* path splitting */
	while (s->p[i].parent != i) {
		int next = s->p[i].parent;
		s->p[i].parent = s->p[next].parent;
		i = next;
	}
	return i;
}

static void sky_djs_union(struct sky *s, int i, int j)
{
	/* union by rank */
	i = sky_djs_find(s, i);
	j = sky_djs_find(s, j);

	/* i and j are in the same set */
	if (i == j)
		return;

	/* ensure rank(i) >= rank(j) */
	if (s->p[i].rank < s->p[j].rank) {
		s->p[i].parent = j;
	} else if (s->p[i].rank > s->p[j].rank) {
		s->p[j].parent = i;
	} else {
		s->p[j].parent = i;
		s->p[i].rank++;
	}
}

static int distance(struct point *a, struct point *b)
{
	int distance = 0;
	for (int i = 0; i < 4; i++)
		distance += (a->v[i] > b->v[i]) ? (a->v[i] - b->v[i]) : (b->v[i] - a->v[i]);
	return distance;
}

static size_t sky_partition(struct sky *s)
{
	for (size_t i = 0; i < s->count; i++) {
		for (size_t j = i + 1; j < s->count; j++) {
			if (distance(s->p+i, s->p+j) <= 3)
				sky_djs_union(s, i, j);
		}
	}

	size_t count = 0;
	for (size_t i = 0; i < s->count; i++)
		if (s->p[i].parent == i)
			count++;

	return count;
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

	struct sky *s = sky_load(input);
	fclose(input);
	if (!s) {
		fprintf(stderr, "Cannot parse the file contents\n");
		return -1;
	}

	sky_partition(s);
	printf("Constellation count: %zu\n", sky_partition(s));
	sky_free(s);
	return 0;
}
