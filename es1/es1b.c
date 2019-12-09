#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct array
{
	int *array;
	size_t size;
	size_t count;
};

static int array_search(struct array *a, int value, size_t *pos)
{
	int low = 0;
	int high = a->count - 1;
	while (low < high) {
		int mid = low + (high - low) / 2;
		if (a->array[mid] < value) {
			low = mid + 1;
		} else {
			high = mid;
		}
	}
	if (pos)
		*pos = high;

	if (a->count && a->array[high] == value)
		return 1;

	return 0;
}

static int array_add(struct array *a, int value)
{
	size_t pos;
	if (array_search(a, value, &pos))
		return -1;

	if (a->size == a->count) {
		size_t newsize = (a->size) ? (a->size * 2) : 1024;
		int *newarray = realloc(a->array, newsize * sizeof(int));
		if (newarray == NULL)
			return -1;

		a->size = newsize;
		a->array = newarray;
	}

	memmove(a->array + pos + 1, a->array + pos, (a->count - pos + 1) * sizeof(int));
	a->array[pos] = value;
	a->count++;
	return 0;
}

static void array_free(struct array *a)
{
	free(a->array);
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		return -1;
	}

	FILE *input = fopen(argv[1], "rb");
	if (input == NULL) {
		fprintf(stderr, "File %s not found\n", argv[1]);
		return -1;
	}

	struct array a = {0};
	int freq = 0;
	array_add(&a, freq);
	while (1) {
		int drift;
		if (fscanf(input, " %d", &drift) == EOF) {
			fseeko(input, 0, 0);
			continue;
		}

		freq += drift;
		if (array_search(&a, freq, NULL))
			break;

		array_add(&a, freq);
	}
	fprintf(stdout, "%zu %zu\n", a.count, a.size);
	array_free(&a);
	fclose(input);

	fprintf(stdout, "Final frequency: %d\n", freq);
	return 0;
}
