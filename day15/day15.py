#!/usr/bin/env python3

import sys

class Unit(object):
    def __init__(self, cls, x, y):
        self.cls = cls
        self.x = x
        self.y = y
        self.hp = 200
        self.attack = 3

    def __lt__(self, other):
        if self.y != other.y:
            return self.y < other.y

        return self.x < other.x

class Map(object):
    def __init__(self):
        self.width = self.height = 0
        self.units = []
        self.data = None
        self.elves = 0
        self.goblins = 0

    def parse(self, txt):
        self.data = []
        self.units = []
        self.elves = 0
        self.goblins = 0
        for y, line in enumerate(txt.splitlines()):
            row = []
            for x, b in enumerate(line):
                if b == 'E':
                    self.units.append(Unit(b, x, y))
                    self.elves += 1
                elif b == 'G':
                    self.units.append(Unit(b, x, y))
                    self.goblins += 1

                if b in ".#GE":
                    row.append(b)

            self.data.append(row)

        self.height = len(self.data)
        self.width = len(self.data[0])

    def adjacent(self, x, y):
        return ((x, y-1), (x-1, y), (x+1, y), (x, y+1))

    def bfs(self, x, y):
        maxdist = self.height * self.width
        dist = [[maxdist for x in range(self.width)] for y in range(self.height)]

        queue = [(x, y)]
        dist[y][x] = 0
        while queue:
            x, y = queue.pop(0)
            for x1, y1 in self.adjacent(x, y):
                if self.data[y1][x1] != '.':
                    continue
                if dist[y1][x1] == maxdist:
                    dist[y1][x1] = dist[y][x] + 1
                    queue.append((x1, y1))
        return dist

    def move(self, unit):
        # check if already in range
        other = unit.cls == "E" and "G" or "E"
        if any(self.data[y][x] == other for x, y in self.adjacent(unit.x, unit.y)):
            return

        # find the destination for the unit
        dest = None
        dist = self.bfs(unit.x, unit.y)
        mindist = self.height * self.width
        for e in self.units:
            if e.hp <= 0 or  e.cls == unit.cls:
                continue
            for x, y in self.adjacent(e.x, e.y):
                if mindist > dist[y][x]:
                    mindist = dist[y][x]
                    dest = (x, y)

        # no destination available
        if dest is None:
            return

        # walk back the path and find the place to move the unit to
        tx, ty = dest
        while True:
            minx, miny = tx, ty
            for x, y in self.adjacent(tx, ty):
                if mindist > dist[y][x]:
                    mindist = dist[y][x]
                    minx, miny = x, y

            if minx == unit.x and miny == unit.y:
                break

            tx, ty = minx, miny

        # update the map and move the unit
        self.data[unit.y][unit.x] = "."
        self.data[ty][tx] = unit.cls
        unit.x, unit.y = tx, ty

    def attack(self, unit):
        minhp = 201
        target = None
        adj = self.adjacent(unit.x, unit.y)
        for e in self.units:
            if e.hp <= 0 or e.cls == unit.cls:
                continue

            for x, y in adj:
                if x == e.x and y == e.y and minhp > e.hp:
                    minhp = e.hp
                    target = e
                    break

        if target:
            target.hp -= unit.attack
            if target.hp <= 0:
                self.data[target.y][target.x] = "."
                return True

        return False

    def get_hp(self):
        return sum(x.hp for x in self.units if x.hp > 0)

    def playgame(self, return_early = False):
        i = 0
        while True:
            self.units.sort()
            for u in self.units:
                if u.hp <= 0:
                    continue
                self.move(u)
                if self.attack(u):
                    if u.cls == "G":
                        self.elves -= 1
                        if return_early or self.elves == 0:
                            return "Goblins", i, self.get_hp()
                    elif u.cls == "E":
                        self.goblins -= 1
                        if self.goblins == 0:
                            return "Elves", i, self.get_hp()

            i += 1

    def __str__(self):
        units = sorted(self.units)
        i = 0
        rows = []
        for y, row in enumerate(self.data):
            data = "".join(row)

            while i < len(units) and units[i].y == y:
                if units[i].hp > 0:
                    data +="  {}({})".format(units[i].cls, units[i].hp)
                i += 1

            rows.append(data)

        return "\n".join(rows)

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

    m = Map()
    m.parse(txt)
    cls, rounds, hp = m.playgame()
    print(m)
    print("Combat ends after {} full rounds".format(rounds))
    print("{} win with {} total hit points left".format(cls, hp))
    print("Outcome: {} * {} = {}".format(rounds, hp, rounds * hp))

    print("\nBisecting the attack level for Elves")
    low = 4
    high = 200
    while low < high:
        mid = (low + high) // 2
        print("Trying with attack {}...".format(mid))
        m.parse(txt)
        for u in m.units:
            if u.cls == 'E':
                u.attack = mid

        cls, rounds, hp = m.playgame(True)
        if m.elves == 0:
            low = mid + 1
        else:
            high = mid

    m.parse(txt)
    for u in m.units:
        if u.cls == 'E':
            u.attack = high
    cls, rounds, hp = m.playgame(True)
    print(m)
    print("Minumum attack needed without dead Elves is {}".format(high))
    print("Combat ends after {} full rounds".format(rounds))
    print("Elves win with {} total hit points left".format(hp))
    print("Outcome: {} * {} = {}".format(rounds, hp, rounds * hp))
