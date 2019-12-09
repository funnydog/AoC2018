#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t reduce(char *str, size_t len)
{
	size_t i = 0;
	while (i < len - 1) {
		if (str[i] != str[i+1] && tolower(str[i]) == tolower(str[i+1])) {
			len -= 2;
			/* copy also the terminator */
			memmove(str + i, str + i + 2, len - i + 1);
			if (i > 0)
				i -= 1;
		} else  {
			i++;
		}
	}
	return len;
}

static size_t strip(char *str, size_t len, int c)
{
	c = tolower(c);
	size_t i = 0;
	while (i < len) {
		if (tolower(str[i]) == c) {
			len--;
			memmove(str + i, str + i + 1, len - i + 1);
		} else {
			i++;
		}
	}
	return len;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		return -1;
	}

	FILE *input = fopen(argv[1], "rb");
	if (!input) {
		fprintf(stderr, "File %s not found\n", argv[1]);
		return -1;
	}

	char *line = NULL;
	size_t bufsize = 0;
	if (getline(&line, &bufsize, input) == -1) {
		free(line);
		fclose(input);
		return -1;
	}
	fclose(input);

	/* chop off the terminal \n */
	size_t linesize = strlen(line) - 1;
	line[linesize] = 0;

	linesize = reduce(line, linesize);
	printf("Answer1: %zu\n", linesize);

	size_t minlen = linesize;
	int chosen = -1;
	for (int c = 'a'; c <= 'z'; c++) {
		char *d = strdup(line);
		if (!d)
			return -1;

		size_t len = strip(d, linesize, c);
		len = reduce(d, len);
		if (len < minlen) {
			minlen = len;
			chosen = c;
		}
		free(d);
	}
	free(line);

	printf("Answer2: %zu by removing '%c'\n", minlen, chosen);

	return 0;
}
