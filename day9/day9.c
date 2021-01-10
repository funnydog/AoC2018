#include <stdio.h>
#include <stdlib.h>

static size_t game(size_t marbles, size_t players)
{
	marbles++;
	size_t *next = malloc(marbles * sizeof(next[0]));
	size_t *prev = malloc(marbles * sizeof(prev[0]));
	size_t *score = calloc(players, sizeof(score[0]));
	if (!next || !prev || !score)
	{
		free(score);
		free(prev);
		free(next);
		return 0;
	}
	next[0] = prev[0] = 0;
	size_t cur = 0;
	size_t player = 0;
	for (size_t i = 1; i < marbles; i++)
	{
		if (i % 23 == 0)
		{
			size_t pos = cur;
			for (int j = 0; j < 7; j++)
			{
				pos = prev[pos];
			}
			next[prev[pos]] = next[pos];
			prev[next[pos]] = prev[pos];

			player %= players;
			score[player] += pos + i;
			cur = next[pos];
		}
		else
		{
			size_t pos1 = next[cur];
			size_t pos2 = next[pos1];
			next[i] = pos2;
			prev[i] = pos1;
			next[pos1] = i;
			prev[pos2] = i;
			cur = i;
		}
		player++;
	}
	size_t max = 0;
	for (size_t i = 0; i < players; i++)
	{
		if (max < score[i])
		{
			max = score[i];
		}
	}
	free(score);
	free(prev);
	free(next);
	return max;
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

	size_t lastmarble, numplayers;
	int r = fscanf(input, " %zu players; last marble is worth %zu points", &numplayers, &lastmarble);
	fclose(input);
	if (r != 2)
	{
		fprintf(stderr, "Cannot parse the data\n");
		return 1;
	}

	printf("Part1: %zu\n", game(lastmarble, numplayers));
	printf("Part2: %zu\n", game(lastmarble*100U, numplayers));
	return 0;
}
