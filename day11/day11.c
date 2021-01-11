#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void build_grid(int grid[301][301], int serial)
{
	memset(grid, 0, sizeof(int)*301*301);
	for (int y = 1; y < 301; y++)
	{
		for (int x = 1; x < 301; x++)
		{
			int rack_id = x + 10;
			int power_level = (rack_id * y + serial) * rack_id;
			grid[y][x] = (power_level / 100) % 10 - 5;

			/* Summed-area table */
			grid[y][x] += grid[y][x-1];
			grid[y][x] += grid[y-1][x];
			grid[y][x] -= grid[y-1][x-1];
		}
	}
}

static int get_pow(int grid[301][301], int x, int y, int s)
{
	/* NOTE: finds the sum from (x+1,y+1) -> (x+side,y+side) */
	return grid[y][x] + grid[y+s][x+s] - grid[y][x+s] - grid[y+s][x];
}

static int find_largest_block(int grid[301][301], int side, int *xm, int *ym)
{
	int maxp = INT_MIN;
	for (int y = 0; y < 301 - side; y++)
	{
		for (int x = 0; x < 301 - side; x++)
		{
			int p = get_pow(grid, x, y, side);
			if (maxp < p)
			{
				maxp = p;
				*xm = x + 1;
				*ym = y + 1;
			}
		}
	}
	return maxp;
}

static int find_largest_square(int grid[301][301], int *side, int *xm, int *ym)
{
	int maxp = INT_MIN;
	for (int s = 1; s <= 300; s++)
	{
		int x, y, p = find_largest_block(grid, s, &x, &y);
		if (maxp < p)
		{
			maxp = p;
			*xm = x;
			*ym = y;
			*side = s;
		}
	}
	return maxp;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return 1;
	}

	FILE *input = fopen(argv[1], "rb");
	if (!input)
	{
		fprintf(stderr, "Cannot open %s\n", argv[1]);
		return 1;
	}
	int serial;
	int r = fscanf(input, " %d", &serial);
	fclose(input);
	if (r != 1)
	{
		fprintf(stderr, "Cannot parse the data\n");
		return 1;
	}

	int grid[301][301];
	build_grid(grid, serial);

	int x, y;
	find_largest_block(grid, 3, &x, &y);
	printf("Part1: %d,%d\n", x, y);

	int s;
	find_largest_square(grid, &s, &x, &y);
	printf("Part2: %d,%d,%d\n", x, y, s);
	return 0;
}
