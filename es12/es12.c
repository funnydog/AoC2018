#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct rule
{
	char input[5];
};

struct state
{
	int *data;
	size_t size;
	size_t count;
};

static int state_find(struct state *s, int val, size_t *pos)
{
	if (!s->count) {
		*pos = 0;
		return 0;
	}

	int low = 0;
	int high = s->count - 1;
	while (low < high) {
		int mid = low + (high - low) / 2;
		if (s->data[mid] < val) {
			low = mid + 1;
		} else {
			high = mid;
		}
	}

	assert(low == high);
	if (s->data[high] == val) {
		*pos = high;
		return 1;
	} else if (s->data[high] < val) {
		*pos = high+1;
		return 0;
	} else {
		assert(high == low);
		*pos = high;
		return 0;
	}
}

static char state_get(struct state *s, int val)
{
	size_t pos;
	if (state_find(s, val, &pos))
		return '#';

	return '.';
}

static void state_set(struct state *s, int val)
{
	size_t pos;
	if (!state_find(s, val, &pos)) {
		if (s->count == s->size) {
			size_t newsize = s->size ? s->size * 2 : 4;
			int *newdata = realloc(s->data, newsize * sizeof(*newdata));
			if (newdata == NULL)
				abort();

			s->data = newdata;
			s->size = newsize;
		}
		memmove(s->data + pos + 1, s->data + pos, (s->count - pos) * sizeof(int));
		s->data[pos] = val;
		s->count++;
	}
}

void state_remove(struct state *s, int val)
{
	size_t pos;
	if (!state_find(s, val, &pos))
		return;

	memmove(s->data+pos, s->data+pos+1, (s->count-pos) * sizeof(int));
	s->count--;
}

int state_eq(struct state *a, struct state *b)
{
	if (a->count != b->count)
		return 0;

	for (size_t i = 0; i < a->count; i++)
		if (a->data[i] != b->data[i])
			return 0;

	return 1;
}

int rule_match(struct rule *r, struct state *s, int pos)
{
	pos -= 2;
	for (int i = 0; i < 5; i++, pos++)
		if (state_get(s, pos) != r->input[i])
			return 0;

	return 1;
}

void state_next(struct state *s, struct state *out, struct rule *rules, size_t rcount)
{
	int low = s->count ? (s->data[0] - 3) : -3;
	int high = s->count ? (s->data[s->count-1] + 3) : 3;
	for (int i = low; i <= high; i++) {
		int found = 0;
		for (struct rule *r = rules; r < rules + rcount; r++) {
			if (rule_match(r, s, i)) {
				state_set(out, i);
				found = 1;
				break;
			}
		}
		if (!found) {
			state_remove(out, i);
		}
	}
}

int state_sum(struct state *s)
{
	int sum = 0;
	for (size_t i = 0; i < s->count; i++) {
		sum += s->data[i];
	}
	return sum;
}

void state_print(struct state *s)
{
	for (size_t i = 0; i < s->count; i++) {
		printf("%d ", s->data[i]);
	}
	printf("\n");
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

	char *line = NULL;
	size_t linesize = 0;
	if (getline(&line, &linesize, input) == -1 ||
	    strstr(line, "initial state: ") != line) {
		fprintf(stderr, "Cannot read the initial state\n");
		fclose(input);
		return -1;
	}

	struct state s = {0};
	char *d = line + strlen("initial state: ");
	for (int i = 0; d[i]; i++) {
		if (d[i] == '#')
			state_set(&s, i);
	}

	struct rule *rules = NULL;
	size_t rsize = 0, rcount = 0;
	while (getline(&line, &linesize, input) != -1) {
		if (strlen(line) >= 10 && line[9] == '#') {
			if (rcount == rsize) {
				size_t newsize = rsize ? rsize * 2 : 2;
				struct rule *newrules = realloc(rules, newsize * sizeof(*rules));
				if (newrules == NULL)
					break;

				rules = newrules;
				rsize = newsize;
			}

			memcpy(rules[rcount].input, line, 5);
			rcount++;
		}
	}
	free(line);

	state_print(&s);
	struct state n = {0};
	uint64_t i;
	int64_t sum = 0;
	int64_t diff = 0;
	for (i = 0; i < 2000u; i++) {
		state_next(&s, &n, rules, rcount);
		struct state t = s;
		s = n;
		n = t;
		int64_t lastsum = sum;
		sum = state_sum(&s);
		diff = sum - lastsum;
		if (i == 19u) {
			printf("Total pots sum for gen %lu: %ld\n", i+1, sum);
		}
	}
	sum += (50000000000 - i) * diff;
	printf("Total pots sum for gen %lu: %ld, %ld\n", i, sum, diff);
	free(s.data);
	free(n.data);

	fclose(input);
	return 0;
}
