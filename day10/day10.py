#!/usr/bin/env python3

import sys
import re

class Light(object):
    def __init__(self, x, y, vx, vy):
        self.x = x
        self.y = y
        self.vx = vx
        self.vy = vy

    def pos(self, time):
        return self.x + self.vx * time, self.y + self.vy * time

def find_range(positions):
    xmin = ymin = 1e12
    xmax = ymax = -1e12
    for x, y in positions:
        if xmin > x: xmin = x
        if xmax < x: xmax = x
        if ymin > y: ymin = y
        if ymax < y: ymax = y

    return xmin, ymin, xmax-xmin+1, ymax-ymin+1

def find_area(lights, time):
    _, _, width, height = find_range(l.pos(time) for l in lights)
    return width * height

def find_local_minimum(lights, a, b):
    # Trisection
    while abs(a - b) > 2:
        u = a + (b-a) // 3
        v = b - (b-a) // 3
        if find_area(lights, u) < find_area(lights, v):
            b = v
        else:
            a = u

    # the solution is in the interval a .. b
    minarea = find_area(lights, b)
    time = b
    for i in range(a, b):
        a = find_area(lights, i)
        if minarea > a:
            minarea = a
            time = i

    return time

def render(lights, time):
    positions = set(l.pos(time) for l in lights)
    xmin, ymin, width, height = find_range(positions)
    lst = []
    for y in range(height):
        lst.append("".join([(x+xmin,y+ymin) in positions and "#" or " " for x in range(width)]))
    return "\n".join(lst)

def parse(txt):
    pattern = re.compile(r"position=<\s*(-?\d+),\s*(-?\d+)> velocity=<\s*(-?\d+),\s*(-?\d+)>")
    return [Light(*(map(int,param))) for param in pattern.findall(txt)]

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} <filename>".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)

    try:
        with open(sys.argv[1], "r") as f:
            txt = f.read().strip()
    except:
        print("Cannot open {} for reading".format(sys.argv[1]), file = sys.stderr)
        exit(-1)

    lights = parse(txt)
    time = find_local_minimum(lights, 0, 100000)
    print("Part1: (see below)")
    print("Part2:", time)
    print(render(lights, time))
