#!/usr/bin/env python3

from functools import cache
import sys
import heapq

NONE = 0
TORCH = 1
GEAR = 2

@cache
def erosion(salt, target, pos):
    if pos == target:
        return salt
    elif pos[0] == 0 or pos[1] == 0:
        return (salt + pos[0] * 16807 + pos[1] * 48271) % 20183
    else:
        return (salt + erosion(salt, target, (pos[0]-1, pos[1])) \
                * erosion(salt, target, (pos[0], pos[1]-1))) % 20183

def rect_risk(depth, target):
    level = 0
    for y in range(target[1]+1):
        for x in range(target[0]+1):
            level += erosion(depth, target, (x, y)) % 3

    return level

def astar(depth, target):
    start = (TORCH, (0,0))
    distance = {}
    queue = []
    heapq.heappush(queue, (0, start))
    distance[start] = 0

    while queue:
        _, cur = heapq.heappop(queue)
        if cur == (TORCH, target):
            return distance[cur]

        ctool, cpos = cur
        cval = erosion(depth, target, cpos) % 3

        for dx, dy in ((0,0), (0,-1), (1,0), (0,1), (-1,0)):
            npos = (cpos[0]+dx, cpos[1]+dy)
            if npos[0] < 0 or npos[1] < 0:
                continue

            nval = erosion(depth, target, npos) % 3

            for ntool in range(3):
                # skip the null and the forbidden movements
                if npos == cpos and ntool == ctool or cval == ntool or nval == ntool:
                    continue

                ndist = distance[cur]
                if npos != cpos:
                    ndist += 1
                if ntool != ctool:
                    ndist += 7

                new = (ntool, npos)
                if new not in distance or ndist < distance[new]:
                    distance[new] = ndist
                    prio = ndist + abs(target[0]-npos[0]) + abs(target[1]-npos[1])
                    heapq.heappush(queue, (prio, new))

    return None

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

    dline, tline = txt.splitlines()
    depth = int(dline.split(" ")[1])
    target = tuple(map(int, tline.split(" ")[1].split(",")))

    print("Part1:", rect_risk(depth, target))
    print("Part2:", astar(depth, target))
