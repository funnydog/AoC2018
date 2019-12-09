#!/usr/bin/env python3

import sys
import re

class Light(object):
    def __init__(self, x, y, vx, vy):
        self.x = x
        self.y = y
        self.vx = vx
        self.vy = vy

    def step_forward(self):
        self.x += self.vx
        self.y += self.vy

    def step_backwards(self):
        self.x -= self.vx
        self.y -= self.vy

class Extents(object):
    def __init__(self):
        self.xmin = 1000000000
        self.ymin = 1000000000
        self.xmax = -1000000000
        self.ymax = -1000000000

    def width(self):
        return self.xmax - self.xmin + 1

    def height(self):
        return self.ymax - self.ymin + 1

def find_area(lights):
    e = Extents()
    for l in lights:
        if e.xmin > l.x:
            e.xmin = l.x
        if e.xmax < l.x:
            e.xmax = l.x
        if e.ymin > l.y:
            e.ymin = l.y
        if e.ymax < l.y:
            e.ymax = l.y
    return e, e.width() * e.height()

pattern = re.compile(r"position=<\s*([-]?\d+),\s*([-]?\d+)> velocity=<\s*([-]?\d+),\s*([-]?\d+)>")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} <filename>".format(sys.argv[0]), file = sys.stderr)
        exit(-1)

    lights = []
    try:
        with open(sys.argv[1], "r") as f:
            for line in f:
                m = pattern.match(line)
                if m:
                    params = [int(m.group(x+1)) for x in range(4)]
                    lights.append(Light(*params))
                else:
                    print(line)
    except:
        print("Cannot open {} for reading".format(sys.argv[1]), file = sys.stderr)
        exit(-1)

    print("Read {} light points".format(len(lights)))

    min_area = 100000000000000000
    min_ext = None
    min_sec = 0
    cur_sec = 0
    while True:
        ext, area = find_area(lights)
        if min_area > area:
            min_area = area
            min_ext = ext
            min_sec = cur_sec
        elif min_area < area:
            break

        for l in lights:
            l.step_forward()

        cur_sec += 1

    print("Minimum reached after {} seconds, area {}".format(min_sec, min_area))
    print("Map size {}x{} starting at ({},{})".format(
        min_ext.width(), min_ext.height(), min_ext.xmin, min_ext.ymin))

    while cur_sec > min_sec:
        cur_sec -= 1
        for l in lights:
            l.step_backwards()

    for y in range(min_ext.ymin, min_ext.ymax+1):
        line = []
        for x in range(min_ext.xmin, min_ext.xmax+1):
            found = False
            for l in lights:
                if l.x == x and l.y == y:
                    found = True
                    line.append("#")
                    break
            if not found:
                line.append(" ")
        print("".join(line))

