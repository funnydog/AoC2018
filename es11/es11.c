#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static void build_grid(int grid[300][300], int serial)
{
	for (int y = 0; y < 300; y++) {
		for (int x = 0; x < 300; x++) {
			int rack_id = x + 1 + 10;
			int power_level = rack_id * (y + 1);
			power_level += serial;
			power_level *= rack_id;
			power_level = (power_level / 100) % 10;
			power_level -= 5;
			grid[y][x] = power_level;
		}
	}
}

static int check_grid(int grid[300][300], int serial, int x, int y, int v)
{
	build_grid(grid, serial);
	if (grid[y-1][x-1] != v) {
		fprintf(stderr, "consistency grid error for grid %d\n", serial);
		return -1;
	}
	return 0;
}

static int find_largest_block(int grid[300][300], int side, int *xm, int *ym)
{
	int xmax, ymax, pmax, pow;
	xmax = ymax = 0;

	/* find the countour condition */
	pow = 0;
	for (int y = 0; y < side; y++) {
		for (int x = 0; x < side; x++) {
			pow += grid[y][x];
		}
	}
	pmax = pow;

	/* compute the values from the old one */
	for (int y = 0; y < 300 - side + 1; y++) {
		int p = pow;
		for (int x = 0; x < 300 - side + 1; x++) {
			if (pmax < p) {
				pmax = p;
				xmax = x;
				ymax = y;
			}

			for (int i = 0; i < side; i++)
				p = p - grid[y+i][x] + grid[y+i][x+side];
		}
		for (int i = 0; i < side; i++)
			pow = pow - grid[y][i] + grid[y+side][i];
	}

	if (xm) *xm = xmax;
	if (ym) *ym = ymax;

	return pmax;
}

static int check_block(int grid[300][300], int serial, int size, int x, int y, int p)
{
	build_grid(grid, serial);
	int xmax, ymax, pmax;
	pmax = find_largest_block(grid, 3, &xmax, &ymax);
	if (x != xmax + 1 || y != ymax + 1 || p != pmax) {
		fprintf(stderr, "consistency block error for grid %d\n", serial);
		return -1;
	}
	return 0;
}

static int find_largest_square(int grid[300][300], int *xm, int *ym, int *side)
{
	int smax, xmax, ymax, pmax = INT_MIN;
	for (int i = 1; i <= 300; i++) {
		int x, y, p = find_largest_block(grid, i, &x, &y);
		if (pmax < p) {
			pmax = p;
			xmax = x;
			ymax = y;
			smax = i;
		}
	}
	if (xm) *xm = xmax;
	if (ym) *ym = ymax;
	if (side) *side = smax;
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

	int grid[300][300];
	assert(check_grid(grid, 8, 3, 5, 4) == 0);
	assert(check_grid(grid, 57, 122, 79, -5) == 0);
	assert(check_grid(grid, 39, 217, 196, 0) == 0);
	assert(check_grid(grid, 71, 101, 153, 4) == 0);
	assert(check_block(grid, 18, 3, 33, 45, 29) == 0);

	build_grid(grid, serial);
	int xmax, ymax, pmax;
	pmax = find_largest_block(grid, 3, &xmax, &ymax);

	printf("Largest total power: %d at (%d,%d)\n", pmax, xmax+1, ymax+1);

	int sqsize;
	pmax = find_largest_square(grid, &xmax, &ymax, &sqsize);
	printf("Largest total power: %d at (%d,%d,%d)\n", pmax, xmax+1, ymax+1, sqsize);
	return 0;
}
