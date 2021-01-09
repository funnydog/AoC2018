#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t react(char *str)
{
	size_t removed = 0;
	size_t i;
	for (i = 0; str[i] == '*'; i++)
	{
		removed++;
	}
	size_t j;
	for (j = i + 1; str[j] == '*'; j++)
	{
		removed++;
	}
	size_t len = strlen(str);
	while (i < len && j < len)
	{
		if (str[i] != str[j] && tolower(str[i]) == tolower(str[j]))
		{
			removed++;
			str[i] = str[j] = '*';
			for (; str[j] == '*'; j++)
			{
				removed++;
			}
			while (i > 0 && str[i] == '*')
			{
				i--;
			}
		}
		else
		{
			for (i = j; str[i] == '*'; i++)
			{
				removed++;
			}
			for (j = i + 1; str[j] == '*'; j++)
			{
				removed++;
			}
		}
	}
	return len-removed;
}

static size_t part1(const char *str)
{
	char *dup = strdup(str);
	size_t rv = react(dup);
	free(dup);
	return rv;
}

static size_t part2(const char *str)
{
	char *copy1 = strdup(str);
	char *copy2 = strdup(str);

	/* first reduction */
	size_t minlen = react(copy1);
	for (int i = 'a'; i <= 'z'; i++)
	{
		for (char *src = copy1, *dst = copy2; *src; src++, dst++)
		{
			*dst = (tolower(*src) == i) ? '*' : *src;
		}
		size_t len = react(copy2);
		if (minlen > len)
		{
			minlen = len;
		}
	}
	free(copy2);
	free(copy1);
	return minlen;
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
		fprintf(stderr, "File %s not found\n", argv[1]);
		return 1;
	}

	char *line = NULL;
	size_t bufsize = 0;
	if (getline(&line, &bufsize, input) == -1)
	{
		free(line);
		fclose(input);
		fprintf(stderr, "Cannot parse the data\n");
		return 1;
	}
	fclose(input);

	/* chop off the terminal \n */
	size_t linesize = strlen(line) - 1;
	line[linesize] = 0;

	printf("Part1: %zu\n", part1(line));
	printf("Part2: %zu\n", part2(line));
	free(line);
	return 0;
}
