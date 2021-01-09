#!/usr/bin/env python3

import sys

def part1(numbers):
    return sum(numbers)

def part2(numbers):
    seen = set()
    freq = 0
    i = 0
    while not freq in seen:
        seen.add(freq)
        freq += numbers[i]
        i += 1
        if i == len(numbers):
            i = 0

    return freq

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} <filename>".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)

    try:
        with open(sys.argv[1], "rt") as f:
            txt = f.read()
    except:
        print("Cannot open {}".format(sts.argv[1]), file=sys.stderr)
        sys.exit(1)

    numbers = tuple(map(int, txt.splitlines()))
    print("Part1:", part1(numbers))
    print("Part2:", part2(numbers))
