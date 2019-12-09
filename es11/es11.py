#!/usr/bin/env python3

import sys

def build_grid(serial):
    grid = [[0 for x in range(301)] for y in range(301)]
    for y in range(1, 301):
        for x in range(1, 301):
            rack_id = x + 10
            power_level = rack_id * y
            power_level += serial
            power_level *= rack_id
            grid[y][x] = (power_level//100) % 10 - 5

            # summed-area table
            grid[y][x] += grid[y][x-1]
            grid[y][x] += grid[y-1][x]
            grid[y][x] -= grid[y-1][x-1]

    return grid

def get_sum(grid, x, y, side):
    return grid[y][x] + grid[y+side][x+side] - grid[y][x + side] - grid[y + side][x]

def find_largest_block(grid, side):
    px, py, pp = 1, 1, grid[1][1]
    for y in range(1, 301-side):
        for x in range(1, 301-side):
            p = get_sum(grid, x, y, side)
            if pp < p:
                pp = p
                px = x
                py = y

    return px, py, pp

def find_largest_block2(grid):
    ps, px, py, pp = 1, 1, 1, grid[0][0]
    for s in range(1,300+1):
        x, y, p = find_largest_block(grid, s)
        if pp < p:
            pp = p
            px = x
            py = y
            ps = s

    return px, py, pp, ps

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} serial".format(sys.argv[0]), file = sys.stderr)
        exit(-1)

    try:
        serial = int(sys.argv[1])
    except:
        print("Cannot parse {}".format(sys.argv[1]), file = sys.stderr)
        exit(-1)

    grid = build_grid(serial)
    x, y, p = find_largest_block(grid, 3)
    print("Largest total power: {} at {},{}".format(p, x+1, y+1))

    x, y, p, s = find_largest_block2(grid)
    print("Largest total power: {} at {},{},{}".format(p, x+1, y+1, s))
