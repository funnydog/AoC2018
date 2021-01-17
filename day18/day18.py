#!/usr/bin/env python3

import sys

OPEN = "."
TREES = "|"
LUMBER = "#"

ADJ = ((0, -1), (1, -1), (1, 0), (1, 1), (0, 1), (-1, 1), (-1, 0), (-1, -1))

def adjacent(area, x, y, width, height):
    trees, lumber = 0, 0
    for dx, dy in ADJ:
        nx = x + dx
        ny = y + dy
        if 0 <= nx < width and 0 <= ny < height:
            if area[ny][nx] == TREES:
                trees += 1
            elif area[ny][nx] == LUMBER:
                lumber += 1
    return trees, lumber

def cycle(area):
    height, width = len(area), len(area[0])
    other = []
    for y, row in enumerate(area):
        nrow = []
        for x, e in enumerate(row):
            trees, lumber = adjacent(area, x, y, width, height)
            if e == OPEN:
                n = (trees >= 3) and TREES or OPEN
            elif e == TREES:
                n = (lumber >= 3) and LUMBER or TREES
            elif e == LUMBER:
                n = (lumber >= 1 and trees >= 1) and LUMBER or OPEN
            else:
                assert 0, "Unknown element at {}".format((x, y))
            nrow.append(n)
        other.append(nrow)
    return other

def resource(area):
    trees = lumber = 0
    for row in area:
        for e in row:
            if e == TREES:
                trees += 1
            elif e == LUMBER:
                lumber += 1
    return trees * lumber

def solve(txt):
    area = [list(row.strip()) for row in txt.splitlines()]
    steps = 1
    tortoise = cycle(area)
    hare = cycle(cycle(area))
    while tortoise != hare:
        if steps == 10:
            part1 = resource(tortoise)
        tortoise = cycle(tortoise)
        hare = cycle(cycle(hare))
        steps += 1

    lam = 1
    hare = cycle(tortoise)
    while tortoise != hare:
        hare = cycle(hare)
        lam += 1

    left = (1000000000 - steps) % lam
    for i in range(left):
        tortoise = cycle(tortoise)

    return part1, resource(tortoise)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} <filename>".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)

    try:
        with open(sys.argv[1], "rt") as f:
            txt = f.read()
    except:
        print("Cannot open {}".format(sys.argv[1]), file=sys.stderr)
        sys.exit(1)

    part1, part2 = solve(txt)
    print("Part1:", part1)
    print("Part2:", part2)
