#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void build_grid(int grid[301][301], int serial)
{
	memset(grid, 0, sizeof(int)*301*301);
	for (int y = 1; y < 301; y++) {
		for (int x = 1; x < 301; x++) {
			int rack_id = x + 10;
			int power_level = rack_id * y;
			power_level += serial;
			power_level *= rack_id;
			power_level = (power_level / 100) % 10;
			power_level -= 5;
			grid[y][x] = power_level;

			/* Summed-area table */
			grid[y][x] += grid[y][x-1];
			grid[y][x] += grid[y-1][x];
			grid[y][x] -= grid[y-1][x-1];
		}
	}
}

static int get_pow(int grid[301][301], int x, int y, int s)
{
	return grid[y][x] + grid[y+s][x+s] - grid[y][x+s] - grid[y+s][x];
}

static int find_largest_block(int grid[301][301], int side, int *xm, int *ym)
{
	int pow = INT_MIN;
	for (int y = 1; y < 301 - side; y++) {
		for (int x = 1; x < 301 - side; x++) {
			int p = get_pow(grid, x, y, side);
			if (pow < p) {
				pow = p;
				if (xm) *xm = x;
				if (ym) *ym = y;
			}
		}
	}
	return pow;
}

static int find_largest_square(int grid[301][301], int *xm, int *ym, int *side)
{
	int pmax = INT_MIN;
	for (int i = 1; i <= 300; i++) {
		int x, y, p = find_largest_block(grid, i, &x, &y);
		if (pmax < p) {
			pmax = p;
			if (xm) *xm = x;
			if (ym) *ym = y;
			if (side) *side = i;
		}
	}
	return pmax;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s serial\n", argv[0]);
		return -1;
	}

	char *end;
	int serial = strtol(argv[1], &end, 10);
	if (end == argv[1] || *end != 0) {
		fprintf(stderr, "Cannot parse the serial %s\n", argv[1]);
		return -1;
	}

	int grid[301][301];
	build_grid(grid, serial);
	int xmax, ymax, pmax;
	pmax = find_largest_block(grid, 3, &xmax, &ymax);

	printf("Largest total power: %d at (%d,%d)\n", pmax, xmax+1, ymax+1);

	int sqsize;
	pmax = find_largest_square(grid, &xmax, &ymax, &sqsize);
	printf("Largest total power: %d at (%d,%d,%d)\n", pmax, xmax+1, ymax+1, sqsize);
	return 0;
}
