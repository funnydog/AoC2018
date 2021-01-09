#!/usr/bin/env python3

import sys

def part1(lines):
    twos = 0
    threes = 0
    for line in lines:
        letters = [0] * (ord('z') - ord('a') + 1)
        for letter in line:
            letters[ord(letter)-ord('a')] += 1

        if any(x == 2 for x in letters):
            twos += 1
        if any(x == 3 for x in letters):
            threes += 1

    return twos * threes

def part2(lines):
    for i, a in enumerate(lines):
        for j in range(i):
            b = lines[j]

            count = 0
            last = 0
            for k in range(len(a)):
                if a[k] != b[k]:
                    last = k
                    count += 1

            if count == 1:
                return a[:last] + a[last+1:]

    return ""

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

    lines = txt.splitlines()
    print("Part1:", part1(lines))
    print("Part2:", part2(lines))
