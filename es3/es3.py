#!/usr/bin/env python3

import re

fabric = [[0 for x in range(1000)] for y in range(1000)]
pattern = re.compile(r"^#(\d+) @ (\d+),(\d+): (\d+)x(\d+)$")
with open("input.txt", "r") as f:
    for line in f:
        m = pattern.match(line)
        if m:
            x1 = int(m.group(2))
            y1 = int(m.group(3))
            w = int(m.group(4))
            h = int(m.group(5))
            for y in range(y1, y1 + h):
                for x in range(x1, x1 + w):
                    fabric[y][x] += 1

square_inches = 0
for y in range(0, 1000):
    for x in range(0, 1000):
        if fabric[y][x] > 1:
            square_inches += 1

print("Square inches:", square_inches)
