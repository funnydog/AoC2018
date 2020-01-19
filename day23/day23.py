#!/usr/bin/env python3

import heapq
import re
import sys

def in_range(a, b):
    return abs(a[0]-b[0]) + abs(a[1]-b[1]) + abs(a[2]-b[2]) <= a[3]

def distance(x, low, high):
    if x < low:
        return low - x
    if x > high:
        return x - high
    return 0

class Box(object):
    def __init__(self, x, y, z, side, bots):
        self.x = x
        self.y = y
        self.z = z
        self.side = side
        self.distance = abs(x) + abs(y) + abs(z)
        self.botcount = 0
        maxx = x + side - 1
        maxy = y + side - 1
        maxz = z + side - 1
        for b in bots:
            d = distance(b[0], x, maxx)
            d += distance(b[1], y, maxy)
            d += distance(b[2], z, maxz)
            if d <= b[3]:
                self.botcount += 1

    def __lt__(self, other):
        if self.botcount != other.botcount:
            return self.botcount > other.botcount
        if self.distance != other.distance:
            return self.distance < other.distance
        return self.side < other.side

def parse(txt):
    r = re.compile(r"pos=<(-?\d+),(-?\d+),(-?\d+)>, r=(\d+)")
    return list(tuple(map(int, bot)) for bot in r.findall(txt))

def part1(bots):
    count = 0
    strongest = max(bots, key = lambda p: p[3])
    for bot in bots:
        if in_range(strongest, bot):
            count += 1
    return count


def part2(bots):
    # find the max extents of the influence of the nanobots
    bmin, bmax = [0]*3, [0]*3
    for b in bots:
        for i in range(3):
            if bmin[i] > b[i] - b[3]: bmin[i] = b[i] - b[3]
            if bmax[i] < b[i] + b[3]: bmax[i] = b[i] + b[3]

    # find the minimum power of two side that covers the area
    # of the influence of the bots for all the axes
    side = 1
    while all(bmin[i] + side < bmax[i] for i in range(3)):
        side *= 2

    # start with a box covering the whole area
    q = []
    heapq.heappush(q, Box(*bmin, side, bots))
    while q:
        box = heapq.heappop(q)
        if box.side == 1:
            break

        # divide the box in 8 boxes and enqueue them in a heap
        # that keeps them ordered by decresing botcount, distance,
        # side
        side = box.side // 2
        for x in range(box.x, box.x + box.side, side):
            for y in range(box.y, box.y + box.side, side):
                for z in range(box.z, box.z + box.side, side):
                    nbox = Box(x, y, z, side, bots)
                    heapq.heappush(q, nbox)

    return box.distance

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

    bots = parse(txt)
    print("Part1:", part1(bots))
    print("Part2:", part2(bots))
