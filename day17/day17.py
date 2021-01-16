#!/usr/bin/env python

import sys
import re

UP    = (0, -1)
RIGHT = (1,  0)
DOWN  = (0,  1)
LEFT  = (-1, 0)

def add(a, b):
    return (a[0]+b[0], a[1]+b[1])

class Map(object):
    def __init__(self, txt):
        vertical = re.compile(r"x=(\d+), y=(\d+)..(\d+)")
        horizontal = re.compile(r"y=(\d+), x=(\d+)..(\d+)")
        self.data = {}
        for line in txt.splitlines():
            m = vertical.match(line)
            if m:
                x, y1, y2 = map(int, m.groups())
                for y in range(y1, y2+1):
                    self.data[x, y] = "#"

            m = horizontal.match(line)
            if m:
                y, x1, x2 = map(int, m.groups())
                for x in range(x1, x2+1):
                    self.data[x, y] = "#"

        self.ymin = 1e12
        self.ymax = 0
        for _, y in self.data.keys():
            if self.ymin > y: self.ymin = y
            if self.ymax < y: self.ymax = y

    def open(self, pos):
        return pos not in self.data or self.data[pos] == "|"

    def fill(self, x, y):
        stack = []
        stack.append((x, y+1))
        while stack:
            pos = stack.pop()
            below = add(pos, DOWN)
            if pos[1] > self.ymax:
                pass
            elif self.data.get(pos, ".") in "~#":
                pass
            elif self.data.get(below, ".") in "~#":
                left = pos
                while self.open(left) and not self.open(add(left, DOWN)):
                    self.data[left] = "|"
                    left = add(left, LEFT)

                right = pos
                while self.open(right) and not self.open(add(right, DOWN)):
                    self.data[right] = "|"
                    right = add(right, RIGHT)

                if self.data.get(left, ".") == "#" and self.data.get(right, ".") == "#":
                    left = add(left, RIGHT)
                    while left != right:
                        self.data[left] = "~"
                        left = add(left, RIGHT)

                    # add the sand above
                    stack.append(add(pos, UP))
                else:
                    stack.append(left)
                    stack.append(right)
            elif pos not in self.data:
                self.data[pos] = "|"
                stack.append(below)

    def count(self):
        sand = water = 0
        for p, t in self.data.items():
            if p[1] < self.ymin:
                pass
            elif t == "|":
                sand += 1
            elif t == "~":
                water += 1
        return sand, water

    def print(self):
        sand = 0
        xmin, xmax = 1e12, -1e12
        for x, _ in self.data.keys():
            if xmin > x: xmin = x
            if xmax < x: xmax = x
        for y in range(self.ymax+1):
            print("".join(self.data.get((x, y), NONE) for x in range(xmin, xmax+1)))

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} <filename>".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)

    try:
        with open(sys.argv[1], "rt") as f:
            txt = f.read().strip()
    except:
        print("Cannot open {}".format(sys.argv[1]), file=sys.stderr)
        sys.exit(1)

    m = Map(txt)
    m.fill(500,0)
    #m.print()
    sand, water = m.count()
    print("Part1:", sand+water)
    print("Part2:", water)
