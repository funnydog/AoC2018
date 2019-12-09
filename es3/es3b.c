#include <stdio.h>
#include <string.h>

struct claim
{
	int id;
	int x;
	int y;
	int w;
	int h;
};

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

	int fabric[1000][1000];
	memset(fabric, 0, sizeof(fabric));

	struct claim c;
	while (fscanf(input, " #%d @ %d,%d: %dx%d", &c.id, &c.x, &c.y, &c.w, &c.h) == 5) {
		for (int i = c.y; i < c.y + c.h; i++) {
			for (int j = c.x; j < c.x + c.w; j++) {
				fabric[i][j]++;
			}
		}
	}
	fseeko(input, 0, 0);
	while (fscanf(input, " #%d @ %d,%d: %dx%d", &c.id, &c.x, &c.y, &c.w, &c.h) == 5) {
		int found = 1;
		for (int i = c.y; i < c.y + c.h; i++) {
			for (int j = c.x; j < c.x + c.w; j++) {
				if (fabric[i][j] > 1) {
					found = 0;
				}
			}
		}
		if (found) {
			printf("ID: %d\n", c.id);
		}
	}

	fclose(input);
}
