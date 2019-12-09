#include <ctype.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s path\n", argv[0]);
		return -1;
	}

	FILE *input = fopen(argv[1], "rb");
	if (!input) {
		fprintf(stderr, "File %s not found\n", argv[1]);
		return -1;
	}

	int letters['z'-'a'+1];
	int two_count = 0;
	int three_count = 0;
	memset(letters, 0, sizeof(letters));
	while (!feof(input)) {
		int c = getc(input);
		if (c == EOF || isspace(c)) {
			int found2, found3;
			found2 = found3 = 0;
			for (int i = 0; i < 'z'-'a'+1; i++) {
				if (letters[i] == 2)
					found2 = 1;
				else if (letters[i] == 3)
					found3 = 1;
			}
			if (found2)
				two_count++;

			if (found3)
				three_count++;

			memset(letters, 0, sizeof(letters));
		} else if (isalpha(c)) {
			letters[tolower(c)-'a']++;
		}
	}
	fclose(input);

	fprintf(stdout, "Checksum: %d\n", two_count * three_count);
	return 0;
}
