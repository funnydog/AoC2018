#include <assert.h>
#include <stdio.h>
#include <string.h>

struct claim
{
	int id;			/* identifier */
	int x;			/* x position */
	int y;			/* y position */
	int w;			/* width */
	int h;			/* height */
};

int fabric[1000][1000];

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

	memset(fabric, 0, sizeof(fabric));

	struct claim c;
	while (fscanf(input, " #%d @ %d,%d: %dx%d", &c.id, &c.x, &c.y, &c.w, &c.h) == 5) {
		for (int y = 0; y < c.h; y++) {
			for (int x = 0; x < c.w; x++) {
				assert(c.y+y >= 0 && c.y+y < 1000);
				assert(c.x+x >= 0 && c.x+x < 1000);
				fabric[c.y+y][c.x+x]++;
			}
		}
	}
	fclose(input);

	int si = 0;
	for (int y = 0; y < 1000; y++) {
		for (int x = 0; x < 1000; x++) {
			if (fabric[x][y]>1)
				si++;
		}
	}

	printf("Square inches: %d\n", si);

	return 0;
}
