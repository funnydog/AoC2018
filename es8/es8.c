#include <stdio.h>
#include <stdlib.h>

struct node
{
	struct node **children;
	size_t ccount;

	size_t *metadata;
	size_t mcount;
};

static void node_free(struct node *n)
{
	for (size_t i = 0; i < n->ccount; i++)
		node_free(n->children[i]);
	free(n);
}

static struct node *node_new(FILE *input)
{
	size_t ccount, mcount;
	if (fscanf(input, " %zu %zu", &ccount, &mcount) != 2)
		return NULL;

	struct node *n = malloc(
		sizeof(*n) +
		ccount * sizeof(n->children[0]) +
		mcount * sizeof(n->metadata[0]));
	if (!n)
		return NULL;

	n->children = (void *)((char *)n + sizeof(*n));
	n->ccount = ccount;
	n->metadata = (void *)((char *)n + sizeof(*n) + ccount * sizeof(n->children[0]));
	n->mcount = mcount;

	for (size_t i = 0; i < ccount; i++)
		n->children[i] = node_new(input);

	for (size_t i = 0; i < mcount; i++) {
		if (fscanf(input, " %zu", &n->metadata[i]) != 1) {
			node_free(n);
			return NULL;
		}
	}

	return n;
}

static int find_license1(struct node *n)
{
	int license = 0;
	for (size_t i = 0; i < n->ccount; i++)
		license += find_license1(n->children[i]);

	for (size_t i = 0; i < n->mcount; i++)
		license += n->metadata[i];

	return license;
}

static int find_license2(struct node *n)
{
	int license = 0;
	if (!n->ccount) {
		license = 0;
		for (size_t i = 0; i < n->mcount; i++) {
			license += n->metadata[i];
		}
		return license;
	} else {
		license = 0;
		for (size_t i = 0; i < n->mcount; i++) {
			size_t j = n->metadata[i];
			if (j > 0 && j <= n->ccount)
				license += find_license2(n->children[j-1]);
		}
		return license;
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [filename]\n", argv[0]);
		return -1;
	}

	FILE *input = fopen(argv[1], "rb");
	if (!input) {
		fprintf(stderr, "Cannot open %s for reading\n", argv[1]);
		return -1;
	}

	struct node *root = node_new(input);
	if (root) {
		printf("License1: %d\n", find_license1(root));
		printf("License2: %d\n", find_license2(root));
		node_free(root);
	}

	fclose(input);
	return 0;
}
