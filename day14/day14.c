#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct recipes
{
	char *data;
	size_t size;
	size_t count;
};

static void append(struct recipes *r, char v)
{
	if (r->count == r->size)
	{
		size_t newsize = r->size ? r->size * 2 : 1024;
		char *newdata = realloc(r->data, newsize * sizeof(newdata[0]));
		if (!newdata)
		{
			abort();
		}
		r->data = newdata;
		r->size = newsize;
	}
	r->data[r->count] = v;
	r->count++;
}

static void make_recipes(char *txt, size_t *part1, size_t *part2)
{
	struct recipes r = {0};
	append(&r, 3);
	append(&r, 7);
	size_t e1 = 0, e2 = 1;

	size_t mlen = strlen(txt);
	if (txt[mlen-1] == '\n')
	{
		mlen--;
	}
	size_t from = 0;
	for (size_t i = 0; i < mlen; i++)
	{
		if (isdigit(txt[i]))
		{
			txt[i] -= '0';
			from = from * 10 + txt[i];
		}
	}

	while (1)
	{
		int n = r.data[e1] + r.data[e2];
		div_t q = div(n, 10);
		if (q.quot)
		{
			append(&r, q.quot);
			if (r.count >= mlen && memcmp(r.data + r.count - mlen, txt, mlen) == 0)
			{
				break;
			}
		}
		append(&r, q.rem);
		if (r.count >= mlen && memcmp(r.data + r.count - mlen, txt, mlen) == 0)
		{
			break;
		}

		e1 = (e1 + r.data[e1] + 1) % r.count;
		e2 = (e2 + r.data[e2] + 1) % r.count;
	}

	*part1 = 0;
	if (r.count >= from+10)
	{
		for (size_t i = 0; i < 10; i++)
		{
			*part1 = *part1 * 10 + r.data[from+i];
		}
	}

	*part2 = (r.count >= mlen) ? r.count - mlen : 0;
	free(r.data);
}


int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return 1;
	}

	FILE *input = fopen(argv[1], "r");
	if (!input)
	{
		fprintf(stderr, "Cannot open %s\n", argv[1]);
		return 1;
	}

	char *line = NULL;
	size_t sline = 0;
	int r = getline(&line, &sline, input);
	fclose(input);
	if (r == -1)
	{
		fprintf(stderr, "Cannot parse the data\n");
		return 1;
	}

	size_t part1, part2;
	make_recipes(line, &part1, &part2);
	free(line);

	printf("Part1: %.010zu\n", part1);
	printf("Part2: %zu\n", part2);
	return 0;
}
