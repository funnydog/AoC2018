#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
	/* class of unit */
	SYSTEM = 0,
	INFECTION,

	/* type of damage */
	RADIATION    = 1<<0,
	BLUDGEONING  = 1<<1,
	FIRE         = 1<<2,
	SLASHING     = 1<<3,
	COLD         = 1<<4,
};

struct group
{
	int index;
	int type;
	int units;
	int hitpoints;
	int attack_damage;
	int attack_type;
	int initiative;

	int weakness;
	int immunity;

	int target;
	int selected;
};

struct game
{
	struct group *pool;
	size_t count;
	size_t size;
};

static int effective_power(const struct group *g)
{
	return g->units * g->attack_damage;
}

static int inflicted_damage(const struct group *a, const struct group *d)
{
	int damage = effective_power(a);
	if ((a->attack_type & d->weakness) != 0)
		damage += damage;
	if ((a->attack_type & d->immunity) != 0)
		damage = 0;
	return damage;
}

static int cmp_power(const void *pa, const void *pb)
{
	const struct group *a = pa, *b = pb;
	int epa = effective_power(a);
	int epb = effective_power(b);
	if (epa != epb)
		return epb - epa;

	return b->initiative - a->initiative;
}

static int cmp_initiative(const void *pa, const void *pb)
{
	const struct group *a = pa, *b = pb;
	return b->initiative - a->initiative;
}

static void game_free(struct game *g)
{
	if (g) {
		free(g->pool);
		free(g);
	}
}

static struct game *game_copy(struct game *g)
{
	struct game *c = malloc(sizeof(*c));
	if (!c)
		return NULL;

	c->size = c->count = g->count;
	c->pool = malloc(c->size * sizeof(c->pool[0]));
	if (!c->pool) {
		free(c);
		return NULL;
	}

	memmove(c->pool, g->pool, c->size * sizeof(c->pool[0]));
	return c;
}

static void game_boost_attack(struct game *g, int type, int boost)
{
	for (size_t i = 0; i < g->count; i++) {
		if (g->pool[i].type == type)
			g->pool[i].attack_damage += boost;
	}
}

static int parse_damage(const char *str)
{
	if (strcmp(str, "radiation") == 0) {
		return RADIATION;
	} else if (strcmp(str, "bludgeoning") == 0) {
		return BLUDGEONING;
	} else if (strcmp(str, "fire") == 0) {
		return FIRE;
	} else if (strcmp(str, "slashing") == 0) {
		return SLASHING;
	} else if (strcmp(str, "cold") == 0) {
		return COLD;
	} else {
		return -1;
	}
}

static struct game *game_load(FILE *input)
{
	struct game *g = calloc(1, sizeof(*g));
	if (!g)
		return NULL;

	char *line = NULL;
	size_t linesize = 0;
	int type = SYSTEM;
	int system_index = 1;
	int infection_index = 1;
	while ((getline(&line, &linesize, input)) != -1) {
		if (strcmp(line, "Immune System:\n") == 0) {
			type = SYSTEM;
			continue;
		} else if (strcmp(line, "Infection:\n") == 0) {
			type = INFECTION;
			continue;
		}

		struct group p = {
			.index = type == SYSTEM ? system_index : infection_index,
			.type = type,
		};
		char *str = line;
		char *tok;
		int i = 0;
		int dtype = 0;
		while ((tok = strsep(&str, " ()\n,;")) != NULL) {
			if (tok[0] == 0) {
				/* skip empty tokens */
			} else if (i == 0) {
				/* units */
				char *end;
				p.units = strtol(tok, &end, 10);
				if (tok != end)
					i++;
			} else if (i == 1) {
				/* hit points */
				char *end;
				p.hitpoints = strtol(tok, &end, 10);
				if (tok != end)
					i++;
			} else if (i == 2) {
				/* weaknesses and immunities */
				if (strcmp(tok, "attack") == 0) {
					i++;
					continue;
				}
				if (strcmp(tok, "weak") == 0) {
					dtype = 0;
					continue;
				}
				if (strcmp(tok, "immune") == 0) {
					dtype = 1;
					continue;
				}
				int damage = parse_damage(tok);
				if (damage <= 0)
					; /* do nothing */
				else if (dtype == 0)
					p.weakness |= damage;
				else
					p.immunity |= damage;
			} else if (i == 3) {
				/* attack damage */
				char *end;
				p.attack_damage = strtol(tok, &end, 10);
				if (tok != end)
					i++;
			} else if (i == 4) {
				/* attack type */
				int damage = parse_damage(tok);
				if (damage > 0) {
					p.attack_type = damage;
					i++;
				}
			} else if (i == 5) {
				/* initiative */
				char *end;
				p.initiative = strtol(tok, &end, 10);
				if (tok != end)
					i++;
			} else {
				break;
			}
		}

		if (i != 6) {
			/* empty or malformed line */
			continue;
		}

		if (type == SYSTEM)
			system_index++;
		else
			infection_index++;

		if (g->count == g->size) {
			size_t newsize = g->size ? g->size * 2 : 2;
			struct group *newpool = realloc(g->pool, newsize * sizeof(newpool[0]));
			if (!newpool) {
				game_free(g);
				return NULL;
			}
			g->size = newsize;
			g->pool = newpool;
		}
		g->pool[g->count++] = p;
	}
	free(line);
	return g;
}

static void game_target_selection(struct game *g)
{
	qsort(g->pool, g->count, sizeof(g->pool[0]), cmp_power);
	for (size_t i = 0; i < g->count; i++) {
		g->pool[i].selected = g->pool[i].target = -1;
	}
	for (size_t i = 0; i < g->count; i++) {
		struct group *pa = g->pool + i;
		if (pa->units <= 0)
			continue;

		int maxdamage = 0;
		struct group *defendant = NULL;
		for (size_t j = 0; j < g->count; j++) {
			struct group *pd = g->pool + j;
			if (pa->type == pd->type || pd->units <= 0 || pd->selected >= 0)
				continue;

			int damage = inflicted_damage(pa, pd);
			if (maxdamage < damage) {
				maxdamage = damage;
				defendant = pd;
			}
		}
		if (defendant) {
			pa->target = defendant->index;
			defendant->selected = pa->index;
		}
	}
}

static void game_attack_phase(struct game *g)
{
	qsort(g->pool, g->count, sizeof(g->pool[0]), cmp_initiative);
	for (size_t i = 0; i < g->count; i++) {
		struct group *pa = g->pool + i;
		if (pa->units <= 0 || pa->target < 0)
			continue;

		/* find the target, linear search */
		struct group *pd = NULL;
		for (size_t j = 0; j < g->count; j++) {
			struct group *t = g->pool + j;
			if (t->type != pa->type && t->index == pa->target) {
				pd = t;
				break;
			}
		}
		if (pd == NULL)
			continue;

		/* inflict the damage */
		int damage = inflicted_damage(pa, pd);
		pd->units -= damage / pd->hitpoints;
	}
}

static int game_end(struct game *g, int *type, int *o)
{
	int outcome = 0;
	int system = 0;
	int infection = 0;
	for (struct group *p = g->pool; p < g->pool + g->count; p++) {
		if (p->units > 0) {
			if (p->type == SYSTEM)
				system++;
			else
				infection++;
			outcome += p->units;
		}
	}

	*o = outcome;
	if (!system) {
		*type = INFECTION;
		return 1;
	} else if (!infection) {
		*type = SYSTEM;
		return 1;
	} else {
		return 0;
	}
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

	struct game *g = game_load(input);
	fclose(input);

	if (!g) {
		fprintf(stderr, "Cannot load the game from file\n");
		return -1;
	}

	struct game *c = game_copy(g);

	int winner, outcome;
	while (!game_end(c, &winner, &outcome)) {
		game_target_selection(c);
		game_attack_phase(c);
	}
	printf("%s wins with %d units left\n",
	       winner == SYSTEM ? "Immune System" : "Infection",
	       outcome);
	game_free(c);

	int low = 0;
	int high = 100000;
	while (low < high) {
		int mid = low + (high - low) / 2;
		c = game_copy(g);
		game_boost_attack(c, SYSTEM, mid);

		int last = -1;
		while  (!game_end(c, &winner, &outcome)) {
			/* exit early if stuck */
			if (last == outcome)
				break;

			last = outcome;
			game_target_selection(c);
			game_attack_phase(c);
		}
		game_free(c);

		if (last == outcome) {
			low++;
		} else if (winner == INFECTION) {
			low = mid + 1;
		} else {
			high = mid;
		}
	}
	game_boost_attack(g, SYSTEM, high);
	while (!game_end(g, &winner, &outcome)) {
		game_target_selection(g);
		game_attack_phase(g);
	}
	printf("With a boost of %d %s wins with %d units left\n",
	       high,
	       winner == SYSTEM ? "Immune System" : "Infection",
	       outcome);

	game_free(g);
	return 0;
}
