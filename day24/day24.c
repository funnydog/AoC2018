#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum
{
	/* class of unit */
	IMMUNE = 0,
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

static int group_power(const struct group *g)
{
	return g->units * g->attack_damage;
}

static int group_damage(const struct group *a, const struct group *d)
{
	int damage = group_power(a);
	if ((a->attack_type & d->immunity) != 0)
	{
		damage = 0;
	}
	else if ((a->attack_type & d->weakness) != 0)
	{
		damage *= 2;
	}
	return damage;
}

static int group_power_cmp(const void *pa, const void *pb)
{
	const struct group *a = pa, *b = pb;
	int epa = group_power(a);
	int epb = group_power(b);
	if (epa != epb)
	{
		return epb - epa;
	}

	return b->initiative - a->initiative;
}

static int group_initiative_cmp(const void *pa, const void *pb)
{
	const struct group *a = pa, *b = pb;
	return b->initiative - a->initiative;
}

static void game_free(struct game *g)
{
	if (g)
	{
		free(g->pool);
		free(g);
	}
}

static struct game *game_clone(struct game *g)
{
	struct game *c = malloc(sizeof(*c));
	if (!c)
	{
		return NULL;
	}

	c->size = c->count = g->count;
	c->pool = malloc(c->size * sizeof(c->pool[0]));
	if (!c->pool)
	{
		free(c);
		return NULL;
	}

	memmove(c->pool, g->pool, c->size * sizeof(c->pool[0]));
	return c;
}

static int parse_damage(const char *str)
{
	static const char *damage[]= {
		"radiation",
		"bludgeoning",
		"fire",
		"slashing",
		"cold",
	};
	for (int i = 0; i < 5; i++)
	{
		if (strcmp(str, damage[i]) == 0)
		{
			return 1<<i;
		}
	}
	return 0;
}

static struct game *game_load(FILE *input)
{
	struct game *g = calloc(1, sizeof(*g));
	if (!g)
	{
		return NULL;
	}

	char *line = NULL;
	size_t linesize = 0;
	int type = IMMUNE;
	while ((getline(&line, &linesize, input)) != -1)
	{
		if (strcmp(line, "Immune System:\n") == 0)
		{
			type = IMMUNE;
			continue;
		}
		else if (strcmp(line, "Infection:\n") == 0)
		{
			type = INFECTION;
			continue;
		}

		struct group p = {
			.index = g->count,
			.type = type,
		};
		char *str = line;
		char *tok;
		int i = 0;
		int dtype = 0;
		while ((tok = strsep(&str, " ()\n,;")) != NULL)
		{
			if (tok[0] == 0)
			{
				/* skip empty tokens */
			}
			else if (i == 0)
			{
				/* units */
				char *end;
				p.units = strtol(tok, &end, 10);
				if (tok != end)
				{
					i++;
				}
			}
			else if (i == 1)
			{
				/* hit points */
				char *end;
				p.hitpoints = strtol(tok, &end, 10);
				if (tok != end)
				{
					i++;
				}
			}
			else if (i == 2)
			{
				/* weaknesses and immunities */
				if (strcmp(tok, "attack") == 0)
				{
					i++;
					continue;
				}
				if (strcmp(tok, "weak") == 0)
				{
					dtype = 0;
					continue;
				}
				if (strcmp(tok, "immune") == 0)
				{
					dtype = 1;
					continue;
				}
				int damage = parse_damage(tok);
				if (dtype == 0)
				{
					p.weakness |= damage;
				}
				else
				{
					p.immunity |= damage;
				}
			}
			else if (i == 3)
			{
				/* attack damage */
				char *end;
				p.attack_damage = strtol(tok, &end, 10);
				if (tok != end)
				{
					i++;
				}
			}
			else if (i == 4)
			{
				/* attack type */
				int damage = parse_damage(tok);
				if (damage > 0)
				{
					p.attack_type = damage;
					i++;
				}
			}
			else if (i == 5)
			{
				/* initiative */
				char *end;
				p.initiative = strtol(tok, &end, 10);
				if (tok != end)
				{
					i++;
				}
			}
			else
			{
				break;
			}
		}

		if (i != 6)
		{
			/* empty or malformed line */
			continue;
		}

		if (g->count == g->size)
		{
			size_t newsize = g->size ? g->size * 2 : 2;
			struct group *newpool = realloc(g->pool, newsize * sizeof(newpool[0]));
			if (!newpool)
			{
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

static int game_play(struct game *g, int boost, size_t *units)
{
	size_t infection = 0;
	for (size_t i = 0; i < g->count; i++)
	{
		if (g->pool[i].type == IMMUNE)
		{
			g->pool[i].attack_damage += boost;
		}
		else
		{
			infection++;
		}
	}

	int *index = malloc(g->count * sizeof(*index));
	assert(index);

	int deadlock = 0;
	while (!deadlock)
	{
		/* target phase */
		qsort(g->pool, g->count, sizeof(g->pool[0]), group_power_cmp);
		for (struct group *a = g->pool; a < g->pool + g->count; a++)
		{
			if (a->units <= 0)
			{
				/* NOTE: skip the dead groups */
				g->count = a - g->pool;
				break;
			}
			a->target = -1;

			int maxdamage = 0;
			struct group *target = NULL;
			for (struct group *b = g->pool; b < g->pool + g->count; b++)
			{
				if (b->type == a->type || b->units <= 0 || b->selected)
				{
					continue;
				}

				int damage = group_damage(a, b);
				if (maxdamage < damage)
				{
					maxdamage = damage;
					target = b;
				}
			}
			if (target)
			{
				a->target = target->index;
				target->selected = 1;
			}
		}

		/* attack phase */
		qsort(g->pool, g->count, sizeof(g->pool[0]), group_initiative_cmp);
		for (size_t i = 0; i < g->count; i++)
		{
			index[g->pool[i].index] = i;
		}

		deadlock = 1;
		for (struct group *a = g->pool; a < g->pool + g->count; a++)
		{
			a->selected = 0;
			if (a->units <= 0 || a->target < 0)
			{
				continue;
			}

			struct group *b = g->pool + index[a->target];
			int dead_units = group_damage(a, b) / b->hitpoints;
			if (dead_units == 0)
			{
				continue;
			}

			deadlock = 0;
			b->units -= dead_units;
			if (b->units <= 0 && b->type == INFECTION)
			{
				infection--;
			}
		}
	}
	free(index);

	if (units)
	{
		*units = 0;
		for (struct group *a = g->pool; a < g->pool + g->count; a++)
		{
			if (a->units > 0)
			{
				(*units) += a->units;
			}
		}
	}
	return infection == 0;
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

	struct game *g = game_load(input);
	fclose(input);

	if (!g)
	{
		fprintf(stderr, "Cannot parse the data\n");
		return 1;
	}

	size_t units;
	struct game *c = game_clone(g);
	if (c)
	{
		game_play(c, 0, &units);
		game_free(c);
		printf("Part1: %zu\n", units);
	}

	int low = 0;
	int high = 100000;
	while (low < high)
	{
		int mid = low + (high - low) / 2;
		c = game_clone(g);
		if (game_play(c, mid, NULL))
		{
			high = mid;
		}
		else
		{
			low = mid+1;
		}
		game_free(c);
	}

	game_play(g, low, &units);
	game_free(g);

	printf("Part2: %zu\n", units);

	return 0;
}
