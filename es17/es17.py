#!/usr/bin/env python

import sys
import re

# constants
NONE = 0
CLAY = 1
SAND = 2
WATER = 3

class Map(object):
    def __init__(self):
        self.xmin = self.ymin = 0
        self.xmax = self.ymax = 0
        self.width = self.height = 0
        self.data = None

    def minmax(self, x, y):
        if self.xmin > x:
            self.xmin = x
        if self.xmax < x:
            self.xmax = x
        if self.ymin > y:
            self.ymin = y
        if self.ymax < y:
            self.ymax = y

    def set(self, x, y, value):
        if x >= self.xmin and x <= self.xmax and y >= self.ymin and y <= self.ymax:
            self.data[y - self.ymin][x - self.xmax] = value

    def get(self, x, y):
        if x >= self.xmin and x <= self.xmax and y >= self.ymin and y <= self.ymax:
            return self.data[y - self.ymin][x - self.xmax]
        return NONE

    def open(self, x, y):
        v = self.get(x, y)
        return v == NONE or v == SAND

    def fill(self, x, y):
        stack = [(x, y)]
        while len(stack) > 0:
            x, y = stack.pop()
            if y > self.ymax:
                pass
            elif not self.open(x, y):
                pass
            elif not self.open(x, y+1):
                l = x
                while self.open(l, y) and not self.open(l, y+1):
                    self.set(l, y, SAND)
                    l -= 1

                r = x + 1
                while self.open(r, y) and not self.open(r, y+1):
                    self.set(r, y, SAND)
                    r += 1

                if self.open(l, y) or self.open(r, y):
                    stack.append((l, y))
                    stack.append((r, y))
                else:
                    l += 1
                    while l < r:
                        self.set(l, y, WATER)
                        l += 1
                    stack.append((x, y-1))
            elif self.get(x, y) == NONE:
                self.set(x, y, SAND)
                stack.append((x, y+1))

    def count(self):
        sand = water = 0
        for y in range(self.height):
            for x in range(self.width):
                if self.data[y][x] == SAND:
                    sand += 1
                elif self.data[y][x] == WATER:
                    water += 1
        return sand, water

    def print(self):
        for y in range(self.height):
            l = []
            for x in range(self.width):
                t = self.data[y][x]
                if t == CLAY:
                    l.append("#")
                elif t == SAND:
                    l.append("|")
                elif t == WATER:
                    l.append("~")
                else:
                    l.append(".")
            print("".join(l))

    def load(self, file):
        vertical = re.compile(r"""x=(\d+), y=(\d+)..(\d+)""")
        horizontal = re.compile(r"""y=(\d+), x=(\d+)..(\d+)""")
        self.xmin = self.ymin = 10000
        self.xmax = self.ymax = -10000
        for line in file:
            m = vertical.match(line)
            if m:
                a, b, c = [int(x) for x in m.groups()]
                self.minmax(a, b)
                self.minmax(a, c)

            m = horizontal.match(line)
            if m:
                a, b, c = [int(x) for x in m.groups()]
                self.minmax(b, a)
                self.minmax(c, a)

        self.xmin -= 1
        self.xmax += 1
        self.width = self.xmax - self.xmin + 1
        self.height = self.ymax - self.ymin + 1
        self.data = [[0 for x in range(self.width)] for y in range(self.height)]

        file.seek(0, 0)
        for line in file:
            m = vertical.match(line)
            if m:
                a, b, c = [int(x) for x in m.groups()]
                for y in range(b, c+1):
                    self.set(a, y, CLAY)

            m = horizontal.match(line)
            if m:
                a, b, c = [int(x) for x in m.groups()]
                for x in range(b, c+1):
                    self.set(x, a, CLAY)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} <filename>".format(sys.argv[0]), file = sys.stderr)
        exit(-1)

    m = Map()
    try:
        with open(sys.argv[1], "rt") as f:
            m.load(f)
    except:
        raise
        print("Cannot open {} for reading".format(sys.argv[1]), file = sys.stderr)
        exit(-1)

    m.fill(500,0)
    # m.print()
    sand, water = m.count()
    print("sand {}, water {}, sum {}".format(sand, water, sand + water))
