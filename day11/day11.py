#!/usr/bin/env python3

import sys

def build_grid(serial):
    grid = [[0 for x in range(301)] for y in range(301)]
    for y in range(1, 301):
        for x in range(1, 301):
            rack_id = x + 10
            power_level = (rack_id * y + serial) * rack_id
            grid[y][x] = (power_level//100) % 10 - 5

            # summed-area table
            grid[y][x] += grid[y][x-1]
            grid[y][x] += grid[y-1][x]
            grid[y][x] -= grid[y-1][x-1]

    return grid

def get_sum(grid, x, y, side):
    # NOTE: finds the sum from (x+1,y+1) -> (x+side,y+side)
    return grid[y][x] + grid[y+side][x+side] - grid[y][x + side] - grid[y + side][x]

def find_largest_block(grid, side):
    px, py = 0, 0
    maxpower = get_sum(grid, 0, 0, side)
    for y in range(301-side):
        for x in range(301-side):
            power = get_sum(grid, x, y, side)
            if maxpower < power:
                maxpower = power
                px, py = x, y

    return px+1, py+1, maxpower

def find_largest_side(grid):
    px, py = 1, 1
    maxpower = -1e12
    maxside = 1
    for s in range(1,301):
        x, y, power = find_largest_block(grid, s)
        if maxpower < power:
            maxpower = power
            maxside = s
            px, py = x, y

    return px, py, maxside, maxpower

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} <input>".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)

    try:
        with open(sys.argv[1], "rt") as f:
            txt = f.read().strip()
    except:
        print("Cannot open {}".format(sys.argv[1]), file=sys.stderr)
        sys.exit(1)

    grid = build_grid(int(txt))
    x, y, _ = find_largest_block(grid, 3)
    print("Part1: {},{}".format(x, y))
    x, y, s, _ = find_largest_side(grid)
    print("Part2: {},{},{}".format(x, y, s))
