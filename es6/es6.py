#!/usr/bin/env python3

import sys

def distance(x0, y0, x1, y1):
    d = 0
    if x1 > x0:
        d += x1 - x0
    else:
        d += x0 - x1

    if y1 > y0:
        d += y1 - y0
    else:
        d += y0 - y1
    return d

class Location:
    def __init__(self, id, x, y):
        self.id = id
        self.x = x
        self.y = y

class Region:
    def __init__(self, locs):
        xmin = 1000000
        ymin = 1000000
        xmax = -1000000
        ymax = -1000000
        for l in locs:
            if xmin > l.x:
                xmin = l.x
            if xmax < l.x:
                xmax = l.x
            if ymin > l.y:
                ymin = l.y
            if ymax < l.y:
                ymax = l.y
        self.x = xmin
        self.y = ymin
        self.w = xmax - xmin + 1
        self.h = ymax - ymin + 1
        self.m = [[0 for x in range(self.w)] for y in range(self.h)]

    def partition(self, locs):
        for y in range(self.h):
            for x in range(self.w):
                label = -1
                mind = 10000
                for l in locs:
                    d = distance(x+self.x, y+self.y, l.x, l.y)
                    if d == mind:
                        label = -1
                    elif d < mind:
                        label = l.id
                        mind = d
                self.m[y][x] = label

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
        self.partition(locs)
        largest = 0
        for l in locs:
            area = self.find_area(l)
            if area > largest:
                largest = area
        return largest

    def compute_distance_sums(self, locs):
        for y in range(self.h):
            for x in range(self.w):
                dsum = 0
                for l in locs:
                    dsum += distance(x+self.x, y+self.y, l.x, l.y)
                self.m[y][x] = dsum

    def find_area_lt(self, limit):
        area = 0
        for y in range(self.h):
            for x in range(self.w):
                if self.m[y][x] < limit:
                    area += 1
        return area

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} filename".format(sys.argv[0]), file = sys.stderr)
        exit(-1)

    locs = []
    try:
        with open(sys.argv[1], "rt") as f:
            for line in f:
                arr = line.split(",")
                id = len(locs)
                x = int(arr[0])
                y = int(arr[1])
                locs.append(Location(id, x, y))
    except Exception as e:
        print("Cannot open {} for reading".format(sys.argv[1]), file = sys.stderr)
        exit(-1)

    r = Region(locs)
    print("Largest area that isn't infinite:", r.find_largest_area(locs))

    r.compute_distance_sums(locs)
    print("Size of the region with distance sum < 32:", r.find_area_lt(32))
    print("Size of the region with distance sum < 10000:", r.find_area_lt(10000))
