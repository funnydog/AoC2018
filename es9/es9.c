#include <stdio.h>
#include <stdlib.h>

struct player
{
	size_t id;
	size_t score;
	struct player *next;
};

struct player *player_new(size_t id)
{
	struct player *p = malloc(sizeof(*p));
	if (p) {
		p->id = id;
		p->score = 0;
		p->next = NULL;
	}
	return p;
}

struct marble
{
	size_t value;
	struct marble *prev, *next;
};

struct marble *marble_new(size_t value)
{
	struct marble *m = malloc(sizeof(*m));
	if (m) {
		m->value = value;
		m->prev = m->next = m;
	}
	return m;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		return -1;
	}

	FILE *input = fopen(argv[1], "rb");
	if (!input) {
		fprintf(stderr, "Cannot open %s for reading\n", argv[1]);
		return -1;
	}

	size_t lastmarble, numplayers;
	if (fscanf(input, " %zu players; last marble is worth %zu points",
		   &numplayers, &lastmarble) != 2) {
		fclose(input);
		return -1;
	}

	struct player *phead = NULL;
	struct player **ptail = &phead;
	for (size_t i = 0; i < numplayers; i++) {
		struct player *p = player_new(i+1);
		if (p) {
			*ptail = p;
			p->next = phead;
			ptail = &p->next;
		}
	}

	struct marble *mhead = marble_new(0);
	struct marble *mcur = mhead;
	struct player *pcur = phead;
	for (size_t i = 1; i < lastmarble + 1; i++) {
		if (i % 23 == 0) {
			struct marble *m = mcur;
			for (int i = 0; i < 7; i++)
				m = m->prev;

			m->next->prev = m->prev;
			m->prev->next = m->next;

			size_t score = m->value + i;
			pcur->score += score;

			mcur = m->next;
			free(m);
		} else {
			struct marble *m = marble_new(i);
			if (m) {
				mcur = mcur->next;
				mcur = mcur->next;

				m->prev = mcur->prev;
				m->next = mcur;
				mcur->prev->next = m;
				mcur->prev = m;

				mcur = m;
			}
		}
		pcur = pcur->next;
	}

	size_t topscore = 0;
	pcur = phead;
	do {
		if (topscore < pcur->score)
			topscore = pcur->score;

		struct player *p = pcur;
		pcur = pcur->next;
		free(p);
	} while (pcur != phead);

	mcur = mhead;
	do {
		struct marble *m = mcur;
		mcur = mcur->next;
		free(m);
	} while (mcur != mhead);

	printf("Top score: %zu\n", topscore);

	fclose(input);
	return 0;
}
