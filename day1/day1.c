#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct hash
{
	int *table;
	size_t capacity;
	size_t count;
};

static struct hash *hash_new(size_t capacity)
{
	struct hash *h = calloc(1, sizeof(h[0]));
	if (h)
	{
		h->table = malloc(capacity * sizeof(h->table[0]));
		if (!h->table)
		{
			free(h);
			return NULL;
		}
		h->capacity = capacity;
		for (size_t i = 0; i < capacity; i++)
		{
			h->table[i] = INT_MAX;
		}
	}
	return h;
}

static void hash_free(struct hash *h)
{
	if (h)
	{
		free(h->table);
		free(h);
	}
}

static int hash_add(struct hash *h, int value)
{
	if (h->count * 100 / h->capacity > 75)
	{
		/* TODO: rehash */
		abort();
	}
	/* TODO: use custom hashfn */
	size_t pos = (unsigned)value % h->capacity;
	for (size_t i = 1; i < h->capacity; i++)
	{
		if (h->table[pos] == INT_MAX)
		{
			break;
		}
		pos = (pos + i*i) % h->capacity;
	}
	h->table[pos] = value;
	h->count++;
	return 1;
}

static int hash_find(struct hash *h, int value)
{
	/* TODO: use custom hashfn */
	size_t pos = (unsigned)value % h->capacity;
	for (size_t i = 1; i < h->capacity; i++)
	{
		if (h->table[pos] == INT_MAX)
		{
			return 0;
		}
		else if (h->table[pos] == value)
		{
			return 1;
		}
		pos = (pos + i*i) % h->capacity;
	}
	return 0;
}

static int part1(const int *numbers, size_t count)
{
	int freq = 0;
	for (size_t i = 0; i < count; i++)
	{
		freq += numbers[i];
	}
	return freq;
}

static int part2(const int *numbers, size_t count)
{
	struct hash *freqs = hash_new(524287U);
	if (!freqs)
	{
		return -1;
	}

	int freq = 0;
	for (;;)
	{
		for (size_t i = 0; i < count; i++)
		{
			hash_add(freqs, freq);
			freq += numbers[i];
			if (hash_find(freqs, freq))
			{
				hash_free(freqs);
				return freq;
			}
		}
	}
}

static int *load(FILE *input, size_t *count)
{
	size_t size = 0;
	int *numbers = NULL, number;
	*count = 0;
	while (fscanf(input, " %d", &number) == 1)
	{
		if (*count == size)
		{
			size_t nsize = size ? size * 2 : 256;
			int *n = realloc(numbers, nsize * sizeof(*n));
			if (!n)
			{
				free(numbers);
				return NULL;
			}
			size = nsize;
			numbers = n;
		}
		numbers[(*count)++] = number;
	}
	return numbers;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		return 1;
	}

	FILE *input = fopen(argv[1], "rb");
	if (!input)
	{
		fprintf(stderr, "Cannot open %s\n", argv[1]);
		return 1;
	}

	size_t count;
	int *numbers = load(input, &count);
	fclose(input);

	if (!numbers)
	{
		fprintf(stderr, "Cannot parse the data\n");
		return 1;
	}

	printf("Part1: %d\n", part1(numbers, count));
	printf("Part2: %d\n", part2(numbers, count));
	free(numbers);
	return 0;
}
