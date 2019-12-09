/* input 077201 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct recipes
{
	int *data;
	size_t size;
	size_t count;
};

static void recipes_init(struct recipes *r)
{
	memset(r, 0, sizeof(*r));
}

static void recipes_destroy(struct recipes *r)
{
	free(r->data);
}

static void recipes_add(struct recipes *r, int v)
{
	if (r->count == r->size) {
		size_t newsize = r->size ? r->size * 2 : 2;
		int *newdata = realloc(r->data, newsize * sizeof(newdata[0]));
		if (!newdata)
			abort();
		r->data = newdata;
		r->size = newsize;
	}
	r->data[r->count] = v;
	r->count++;
}

static void recipes_print(struct recipes *r, size_t e1, size_t e2)
{
	for (size_t j = 0; j < r->count; j++) {
		if (j == e1) {
			printf("(%d)", r->data[j]);
		} else if (j == e2) {
			printf("[%d]", r->data[j]);
		} else {
			printf(" %d ", r->data[j]);
		}
	}
	printf("\n");
}

static int *intcmp(int *haystack, size_t hs, int *needle, size_t ns)
{
	for (size_t i = 0; i < hs; i++) {
		size_t j;
		for (j = 0; j < ns; j++) {
			if (haystack[i+j] != needle[j])
				break;
		}
		if (j == ns) {
			return haystack + i;
		}
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <number>\n", argv[0]);
		return -1;
	}

	char *end;
	size_t from = strtoul(argv[1], &end, 10);
	if (end == argv[1] || *end != '\0') {
		fprintf(stderr, "Cannot parse the number %s\n", argv[1]);
		return -1;
	}

	size_t numlen = strlen(argv[1]);
	int *numbers = malloc(sizeof(numbers[0]) * numlen);
	for (size_t i = 0; i < numlen; i++)
		numbers[i] = argv[1][i] - '0';

	struct recipes r = {0};
	recipes_init(&r);
	recipes_add(&r, 3);
	recipes_add(&r, 7);
	size_t e1 = 0, e2 = 1, pos = 0;
	size_t i;
	for (i = 0; pos < numlen; i++) {
		int newrecipe = r.data[e1] + r.data[e2];
		int digit;
		if (newrecipe >= 10) {
			digit = newrecipe / 10;
			recipes_add(&r, digit);
			if (digit == numbers[pos]) {
				pos++;
			} else if (digit == numbers[0]) {
				pos = 1;
			} else {
				pos = 0;
			}
			if (pos == numlen)
				break;
		}
		digit = newrecipe % 10;
		recipes_add(&r, digit);
		if (digit == numbers[pos]) {
			pos++;
		} else if (digit == numbers[0]) {
			pos = 1;
		} else {
			pos = 0;
		}

		e1 = (e1 + r.data[e1] + 1) % (r.count);
		e2 = (e2 + r.data[e2] + 1) % (r.count);
//		recipes_print(&r, e1, e2);
	}
	free(numbers);

	printf("Recipes at the right: ");
	for (size_t i = 0; i < 10; i++) {
		printf("%d", r.data[i+from]);
	}
	printf("\n");

	printf("Num of recipes at the left: %zu\n", r.count - numlen);
	printf("Num of iterations: %zu\n", i);

	recipes_destroy(&r);
	return 0;
}
