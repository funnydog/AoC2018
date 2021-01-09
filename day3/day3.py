#!/usr/bin/env python3

import re
import sys

def parse(txt):
    lst = []
    pattern = re.compile(r"^#(\d+) @ (\d+),(\d+): (\d+)x(\d+)$")
    for line in txt.splitlines():
        m = pattern.match(line)
        if m:
            lst.append(tuple(map(int, m.groups())))
    return lst

def project(claim):
    height = max(cx+cw for _,cx,_,cw,_ in claims)+1
    width = max(cy+ch for _,_,cy,_,ch in claims)+1
    fabric = [[0 for x in range(width)] for y in range(height)]
    for _, cx, cy, cw, ch in claims:
        for y in range(cy, cy+ch):
            for x in range(cx, cx+cw):
                fabric[y][x] += 1

    return fabric

def part1(fabric):
    return sum(sum(1 for val in row if val > 1) for row in fabric)

def part2(fabric, claims):
    for cid, cx, cy, cw, ch in claims:
        if all(fabric[y][x] == 1 for x in range(cx, cx+cw) for y in range(cy, cy+ch)):
            return cid
    return None

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} <filename>".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)

    try:
        with open(sys.argv[1], "rt") as f:
            txt = f.read()
    except:
        print("Cannot open {}".format(sys.argv[1]))
        sys.exit(1)

    claims = parse(txt)
    fabric = project(claims)
    print("Part1:", part1(fabric))
    print("Part2:", part2(fabric, claims))
