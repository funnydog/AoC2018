#!/usr/bin/env python3

import sys

def recipes(txt):
    numbers = list(map(int, list(txt)))
    mlen = len(numbers)
    pos = 0

    recipes = [3, 7]
    e1, e2 = 0, 1

    while True:
        n = recipes[e1] + recipes[e2];
        q, r = divmod(n, 10)
        if q:
            recipes.append(q)
            if recipes[-mlen:] == numbers:
                break
        recipes.append(r)
        if recipes[-mlen:] == numbers:
            break

        e1 = (e1 + recipes[e1] + 1) % len(recipes)
        e2 = (e2 + recipes[e2] + 1) % len(recipes)

    start = int(txt)
    part1 = "";
    if len(recipes) >= start + 10:
        part1 = "".join(map(str, recipes[start:start+10]))
    part2 = len(recipes) - mlen
    if part2 < 0:
        part2 = 0
    return part1, part2

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} <filename>".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)

    try:
        with open(sys.argv[1], "rt") as f:
            txt = f.readline().strip()
    except FileNotFoundError:
        print("Cannot open {}".format(sys.argv[1]), file=sys.stderr)
        sys.exit(1)

    part1, part2 = recipes(txt)
    print("Part1:", part1)
    print("Part2:", part2)
