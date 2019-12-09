#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

static int difference(char *line1, char *line2)
{
	int d = 0;
	while (*line1 && *line2) {
		if (*line1 != *line2)
			d++;

		line1++;
		line2++;
	}
	return d;
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

	size_t lines_size = 256;
	size_t lines_count = 0;
	char **lines = malloc(lines_size * sizeof(char *));
	if (lines == NULL) {
		fprintf(stderr, "OOM: malloc\n");
		fclose(input);
		return -1;
	}

	char *line = NULL;
	size_t line_length = 0;
	while (getline(&line, &line_length, input) != -1) {
		if (lines_count == lines_size) {
			size_t newsize = lines_size *2;
			char **newlines = realloc(lines, sizeof(char *) * newsize);
			if (newlines == NULL) {
				fprintf(stderr, "OOM: realloc\n");
				return -1;
			}
			lines_size = newsize;
			lines = newlines;
		}
		lines[lines_count++] = line;
		line = NULL;
		line_length = 0;
	}
	free(line);
	fclose(input);

	int mind = 26;
	char *w1, *w2;
	w1 = w2 = NULL;
	for (size_t i = 0; i < lines_count - 1; i++) {
		for (size_t j = i + 1; j < lines_count; j++) {
			int d = difference(lines[i], lines[j]);
			if (mind > d) {
				mind = d;
				w1 = lines[i];
				w2 = lines[j];
			}
		}
	}
	if (w1 != NULL && w2 != NULL) {
		while (*w1 != 0 && *w2 != 0) {
			if (*w1 == *w2) {
				printf("%c", *w1);
			}
			w1++;
			w2++;
		}
	}

	for (size_t i = 0; i < lines_count; i++)
		free(lines[i]);
	free(lines);
	return 0;
}
