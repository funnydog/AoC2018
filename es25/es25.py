#!/usr/bin/env python3

import sys

class Point(object):
    def __init__(self, values, index):
        self.v = values
        self.parent = index
        self.rank = 0

def distance(p1, p2):
    d = 0
    for i in range(4):
        if p1.v[i] > p2.v[i]:
            d += p1.v[i] - p2.v[i]
        else:
            d += p2.v[i] - p1.v[i]

    return d

class Sky(object):
    def __init__(self):
        self.points = []

    def load(self, f):
        i = 0
        for line in f:
            val = [int(x) for x in line.strip().split(",")]
            self.points.append(Point(val, i))
            i += 1

    def find(self, i):
        while self.points[i].parent != i:
            next = self.points[i].parent
            self.points[i].parent = self.points[next].parent
            i = next

        return i

    def merge(self, i, j):
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
                    self.merge(i, j)

        count = 0
        for i in range(len(self.points)):
            if self.points[i].parent == i:
                count += 1

        return count

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} <filename>".format(sys.argv[0]), file = sys.stderr)
        exit(-1)

    s = Sky()
    try:
        with open(sys.argv[1], "rt") as f:
            s.load(f)
    except:
        raise
        print("Cannot open {} for reading".format(sys.argv[1]), file = sys.stderr)
        exit(-1)

    print("Constellation count: {}".format(s.count_constellations()))
