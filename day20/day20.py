#!/usr/bin/env python3

import sys

DIR = {
    "N": (0, -1),
    "E": (1, 0),
    "S": (0, 1),
    "W": (-1, 0),
}

class Map(object):
    def __init__(self, txt):
        self.x = 0
        self.y = 0
        self.data = {}
        self.walk(list(txt))

    def walk(self, lst):
        ox, oy = self.x, self.y
        while lst:
            v = lst.pop(0)
            if v in ")$":
                break
            elif v == "^":
                self.x = self.y = 0
                self.data[self.x, self.y] = "X"
            elif v == "|":
                self.x, self.y = ox, oy
            elif v == "(":
                self.walk(lst)
            elif v in DIR:
                dx, dy = DIR[v]
                self.x += dx
                self.y += dy
                # door
                self.data[self.x, self.y] = dx == 0 and "-" or "|"
                self.x += dx
                self.y += dy
                # room
                self.data[self.x, self.y] = "."
            else:
                assert 0, "Unknown symbol {}".format(v)

    def bfs(self, x, y):
        cost = {}
        queue = []
        queue.append((x, y))
        cost[x, y] = 0
        while queue:
            x, y = queue.pop(0)
            for dx, dy in ((0, -1), (1, 0), (0, 1), (-1, 0)):
                nx = x + dx
                ny = y + dy
                if (nx, ny) not in self.data:
                    continue

                ncost = cost[x, y] + 1
                if (nx, ny) not in cost:
                    cost[nx, ny] = ncost
                    queue.append((nx, ny))

        part1 = max(cost.values()) // 2
        part2 = sum(1 for k, c in cost.items() if c >= 2000 and self.data[k] == ".")
        return part1, part2

    def __repr__(self):
        ymin = xmin = 1e12
        ymax = xmax = -1e12
        for x, y in self.data.keys():
            if xmin > x: xmin = x
            if xmax < x: xmax = x
            if ymin > y: ymin = y
            if ymax < y: ymax = y

        s = ""
        for y in range(ymin-1, ymax+2):
            for x in range(xmin-1, xmax+2):
                s += self.data.get((x, y), "#")
            s += "\n"
        return s

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

    m = Map(txt)
    #print(m)
    part1, part2 = m.bfs(0, 0)
    print("Part1:", part1)
    print("Part2:", part2)
