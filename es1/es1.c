#include <ctype.h>
#include <stdio.h>

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

	int freq = 0;
	int drift = 0;
	while (1) {
		if (fscanf(input, " %d", &drift) == EOF)
			break;
		freq += drift;
		drift = 0;
	}
	fclose(input);

	fprintf(stdout, "Final frequency: %d\n", freq);
	return 0;
}
