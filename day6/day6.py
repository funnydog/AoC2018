#!/usr/bin/env python3

import sys

def distance(x, y):
    return abs(x)+abs(y)

class Location:
    def __init__(self, id, x, y):
        self.id = id
        self.x = x
        self.y = y

class Region:
    def __init__(self, locs):
        xmin = ymin = 1e12
        xmax = ymax = -1e12
        for l in locs:
            if xmin > l.x: xmin = l.x
            if xmax < l.x: xmax = l.x
            if ymin > l.y: ymin = l.y
            if ymax < l.y: ymax = l.y

        self.x = xmin
        self.y = ymin
        self.w = xmax - xmin + 1
        self.h = ymax - ymin + 1
        self.m = [[0 for x in range(self.w)] for y in range(self.h)]

    def find_area(self, loc):
        area = 0
        for y in range(self.h):
            for x in range(self.w):
                if self.m[y][x] == loc.id:
                    if x == 0 or x == self.w-1 or y == 0 or y == self.h-1:
                        return 0
                    area += 1

        return area

    def find_largest_area(self, locs):
        for y in range(self.h):
            for x in range(self.w):
                label = -1
                mind = 1e12
                for l in locs:
                    d = distance(x+self.x-l.x, y+self.y-l.y)
                    if d == mind:
                        label = -1
                    elif mind > d:
                        mind = d
                        label = l.id
                self.m[y][x] = label

        largest = 0
        for l in locs:
            area = self.find_area(l)
            if largest < area:
                largest = area
        return largest

    def find_area_lt(self, locs, limit):
        for y in range(self.h):
            for x in range(self.w):
                dsum = 0
                for l in locs:
                    dsum += distance(x+self.x-l.x, y+self.y-l.y)
                self.m[y][x] = dsum

        area = 0
        for y in range(self.h):
            for x in range(self.w):
                if self.m[y][x] < limit:
                    area += 1
        return area

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} filename".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)

    try:
        with open(sys.argv[1], "rt") as f:
            txt = f.read()
    except Exception as e:
        print("Cannot open {} for reading".format(sys.argv[1]), file=sys.stderr)
        sys.exit(1)

    locs = []
    for line in txt.splitlines():
        x, y = map(int, line.split(","))
        id = len(locs)
        locs.append(Location(id, x, y))

    r = Region(locs)
    print("Part1:", r.find_largest_area(locs))
    print("Part2:", r.find_area_lt(locs, 10000))
