#!/usr/bin/env python3

import sys

class Point(object):
    def __init__(self, values, index):
        self.v = values
        self.parent = index
        self.rank = 0

def distance(p1, p2):
    return sum(abs(a - b) for a, b in zip(p1.v, p2.v))

class Sky(object):
    def __init__(self, txt):
        self.points = []
        for i, line in enumerate(txt.splitlines()):
            val = tuple(map(int, line.strip().split(",")))
            self.points.append(Point(val, i))

    def find(self, i):
        while self.points[i].parent != i:
            next = self.points[i].parent
            self.points[i].parent = self.points[next].parent
            i = next

        return i

    def union(self, i, j):
        i = self.find(i)
        j = self.find(j)

        if i == j:
            pass
        elif self.points[i].rank < self.points[j].rank:
            self.points[i].parent = j
        elif self.points[i].rank > self.points[j].rank:
            self.points[j].parent = i
        else:
            self.points[j].parent = i
            self.points[i].rank += 1

    def count_constellations(self):
        for i in range(len(self.points)):
            for j in range(i):
                if distance(self.points[i], self.points[j]) <= 3:
                    self.union(i, j)

        count = 0
        for i, point in enumerate(self.points):
            if point.parent == i:
                count += 1

        return count

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

    s = Sky(txt)
    print("Part1:", s.count_constellations())
