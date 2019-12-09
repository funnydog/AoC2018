#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct map
{
	char **data[2];
	int cur;
	size_t width;
	size_t height;
	size_t size;
};


static void map_free(struct map *m)
{
	if (m) {
		while (m->height-- > 0) {
			free(m->data[0][m->height]);
			free(m->data[1][m->height]);
		}
		free(m->data[1]);
		free(m->data[0]);
		free(m);
	}
}

static struct map *map_load(FILE *input)
{
	struct map *m = calloc(1, sizeof(*m));
	if (!m)
		return NULL;

	char *line = NULL;
	size_t linesize = 0;
	while (getline(&line, &linesize, input) != -1) {
		if (!m->width) {
			m->width = strlen(line)-1;
		}
		line[m->width] = 0;
		if (m->height == m->size) {
			size_t newsize = m->size ? m->size * 2 : 2;
			for (int i = 0; i < 2; i++) {
				char **newdata = realloc(m->data[i], newsize * sizeof(newdata[0]));
				if (!newdata) {
					map_free(m);
					return NULL;
				}
				m->data[i] = newdata;
			}
			m->size = newsize;
		}
		m->data[0][m->height] = strdup(line);
		m->data[1][m->height] = strdup(line);
		m->height++;
		if (!m->data[0] || !m->data[1]) {
			map_free(m);
			return NULL;
		}
	}
	free(line);

	return m;
}

static int map_get(struct map *m, int x, int y)
{
	if (x >= 0 && (size_t)x < m->width && y >= 0 && (size_t)y < m->height)
		return m->data[m->cur][y][x];

	return ' ';
}

static void map_set(struct map *m, int x, int y, int value)
{
	if (x >= 0 && (size_t)x < m->width && y >= 0 && (size_t)y < m->height)
		m->data[1-m->cur][y][x] = value;
}

static void map_swapbuf(struct map *m)
{
	m->cur = 1 - m->cur;
}

static void map_adjacent(struct map *m, int x, int y, int *open, int *wood, int *lumber)
{
	const int adj[][2] = {
		{ -1, -1 },
		{  0, -1 },
		{  1, -1 },
		{ -1,  0 },
		{  1,  0 },
		{ -1,  1 },
		{  0,  1 },
		{  1,  1 },
	};
	*open = *wood = *lumber = 0;
	for (int i = 0; i < 8; i++) {
		switch (map_get(m, x + adj[i][0], y + adj[i][1])) {
		case '.': (*open)++; break;
		case '|': (*wood)++; break;
		case '#': (*lumber)++; break;
		}
	}
}

static void map_update(struct map *m)
{
	for (size_t y = 0; y < m->height; y++) {
		for (size_t x = 0; x < m->width; x++) {
			int open, wood, lumber;
			map_adjacent(m, x, y, &open, &wood, &lumber);
			switch (map_get(m, x, y)) {
			case '.':
				map_set(m, x, y, (wood >= 3) ? '|' : '.');
				break;
			case '|':
				map_set(m, x, y, (lumber >= 3) ? '#' : '|');
				break;
			case '#':
				map_set(m, x, y, (lumber >= 1 && wood >= 1) ? '#' : '.');
				break;
			}
		}
	}
	map_swapbuf(m);
}

static int map_get_resources(struct map *m)
{
	int w = 0, l = 0;
	for (size_t y = 0; y < m->height; y++) {
		for (size_t x = 0; x < m->width; x++) {
			switch (m->data[m->cur][y][x]) {
			case '|': w++; break;
			case '#': l++; break;
			}
		}
	}
	return w * l;
}

static void map_print(struct map *m)
{
	for (size_t y = 0; y < m->height; y++) {
		printf("%s\n", m->data[m->cur][y]);
	}
}

struct pair
{
	int minute;
	int value;
};

struct stack
{
	struct pair *pool;
	size_t count;
	size_t size;
};

static void stack_push(struct stack *s, int minute, int value)
{
	if (s->count == s->size) {
		size_t newsize = s->size ? s->size * 2 : 2;
		struct pair *p = realloc(s->pool, newsize * sizeof(p[0]));
		if (p == NULL)
			abort();

		s->size = newsize;
		s->pool = p;
	}
	s->pool[s->count].minute = minute;
	s->pool[s->count].value = value;
	s->count++;
}

static struct pair stack_get(struct stack *s)
{
	return s->pool[s->count-1];
}

static void stack_pop(struct stack *s)
{
	s->count--;
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
	struct map *m = map_load(input);
	fclose(input);
	if (!m) {
		fprintf(stderr, "Cannot load the map\n");
		return -1;
	}

	printf("Initial state\n");
	map_print(m);
	printf("\n");

	struct stack s = {0};
	int minute, period = 1;
	for (minute = 1; minute <= 10000; minute++) {
		map_update(m);
		int value = map_get_resources(m);
		if (minute == 10) {
			printf("Resources after          10 minutes: %d\n", value);
		}
		while (s.count) {
			struct pair p = stack_get(&s);
			if (p.value == value) {
				period = minute - p.minute;
				break;
			}

			if (p.value < value)
				break;

			stack_pop(&s);
		}
		stack_push(&s, minute, value);
	}

	if (s.count) {
		struct pair p = stack_get(&s);
		size_t i = (1000000000u - p.minute) % period;
		while (i-- > 0)
			map_update(m);

		int res = map_get_resources(m);
		printf("Resources after 10000000000 minutes: %d\n", res);
	}
	free(s.pool);
	map_free(m);
	return 0;
}
