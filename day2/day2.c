#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct boxid
{
	struct boxid *next;
	char id[0];
};

static struct boxid *load(FILE *input)
{
	struct boxid *head = NULL;
	struct boxid **tail = &head;

	size_t size = 0;
	char *line = NULL;
	while (getline(&line, &size, input)!=-1)
	{
		/* chop off the trailing \n */
		size_t len = strlen(line);
		if (line[len-1] == '\n')
		{
			line[len-1] = 0;
			len--;
		}

		/* add the string to the list */
		struct boxid *b = malloc(len+1+sizeof(*b));
		if (!b)
		{
			break;
		}
		strcpy(b->id, line);
		b->next = NULL;
		*tail = b;
		tail = &b->next;
	}
	free(line);
	return head;
}

static int part1(struct boxid *lst)
{
	size_t twos = 0;
	size_t threes = 0;
	size_t count['z'-'a'+1];
	for (; lst; lst = lst->next)
	{
		memset(count, 0, sizeof(count));
		for (char *s = lst->id; *s; s++)
		{
			count[*s-'a']++;
		}
		int found2 = 0;
		int found3 = 0;
		for (size_t i = 0; i < 'z'-'a'+1; i++)
		{
			switch (count[i])
			{
			case 2: found2 = 1; break;
			case 3: found3 = 1; break;
			}
		}
		if (found2) twos++;
		if (found3) threes++;
	}
	return twos * threes;
}

static char *part2(struct boxid *lst)
{
	for (struct boxid *a = lst->next; a; a = a->next)
	{
		for (struct boxid *b = lst; b != a; b = b->next)
		{
			char *sa = a->id;
			char *sb = b->id;
			size_t last = 0;
			size_t count = 0;
			size_t i;
			for (i = 0; *sa && *sb; i++)
			{
				if (*sa++ != *sb++)
				{
					count++;
					last = i;
				}
			}
			if (count == 1)
			{
				memmove(b->id+last, b->id+last+1, i-last);
				return b->id;
			}
		}
	}
	return "";
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s path\n", argv[0]);
		return 1;
	}

	FILE *input = fopen(argv[1], "rb");
	if (!input)
	{
		fprintf(stderr, "Cannot open %s\n", argv[1]);
		return 1;
	}

	struct boxid *lst = load(input);
	fclose(input);
	if (!lst)
	{
		fprintf(stderr, "Cannot parse the data\n");
		return 1;
	}

	printf("Part1: %d\n", part1(lst));
	printf("Part2: %s\n", part2(lst));
	while (lst)
	{
		struct boxid *tmp = lst;
		lst = lst->next;
		free(tmp);
	}

	return 0;
}
