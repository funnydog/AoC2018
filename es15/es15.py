#!/usr/bin/env python3

import sys
import copy

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
    CLSDSC = {"E": "Elves", "G": "Goblins"}

    def __init__(self):
        self.width = self.height = 0
        self.units = []
        self.data = None
        self.dist = None

    def load(self, f):
        self.data = []
        self.units = []
        y = 0
        for line in f:
            row = []
            x = 0
            for b in line:
                if b == 'E' or b == 'G':
                    self.units.append(Unit(b, x, y))
                    b = '.'
                if b == '.' or b == '#':
                    row.append(b)
                    x += 1

            self.data.append(row)
            y += 1

        self.height = len(self.data)
        self.width = len(self.data[0])

    def adjacent(self, x, y):
        return ((x, y-1), (x-1, y), (x+1, y), (x, y+1))

    def place_units(self):
        for u in self.units:
            if u.hp > 0:
                self.data[u.y][u.x] = u.cls

    def remove_units(self):
        for u in self.units:
            if u.hp > 0:
                self.data[u.y][u.x] = '.'

    def bfs(self, x, y):
        self.place_units()

        maxdist = self.height * self.width
        self.dist = [[maxdist for x in range(self.width)] for y in range(self.height)]

        queue = [(x, y)]
        self.dist[y][x] = 0
        while len(queue) > 0:
            x, y = queue.pop(0)
            for x1, y1 in self.adjacent(x, y):
                if self.data[y1][x1] != '.':
                    continue
                if self.dist[y1][x1] == maxdist:
                    self.dist[y1][x1] = self.dist[y][x] + 1
                    queue.append((x1, y1))

        self.remove_units()

    def find_target(self, unit):
        self.bfs(unit.x, unit.y)
        mindist = self.height * self.width
        target = None
        for e in self.units:
            if e.cls != unit.cls and e.hp > 0:
                for x, y in self.adjacent(e.x, e.y):
                    if mindist > self.dist[y][x]:
                        mindist = self.dist[y][x]
                        target = (x, y)

        return target

    def move(self, unit):
        stop = True
        for e in self.units:
            if e.hp > 0 and e.cls != unit.cls:
                stop = False
                break

        if stop:
            return False

        target = self.find_target(unit)
        if target is None:
            return True

        tx, ty = target
        if tx == unit.x and ty == unit.y:
            return True

        while True:
            mindist = self.width * self.height
            minx, miny = tx, ty
            for x, y in self.adjacent(tx, ty):
                if mindist > self.dist[y][x]:
                    mindist = self.dist[y][x]
                    minx, miny = x, y

            if minx == unit.x and miny == unit.y:
                break

            tx, ty = minx, miny

        unit.x = tx
        unit.y = ty
        return True

    def attack(self, unit):
        minhp = 201
        target = None
        adj = self.adjacent(unit.x, unit.y)
        for e in self.units:
            if e.cls != unit.cls and e.hp > 0:
                for x, y in adj:
                    if x == e.x and y == e.y and minhp > e.hp:
                        minhp = e.hp
                        target = e
                        break

        if target:
            target.hp -= unit.attack

    def playround(self):
        self.units.sort()
        for u in self.units:
            if u.hp > 0:
                if not self.move(u):
                    return False

                self.attack(u)

        return True

    def get_survivors_cls(self):
        for u in self.units:
            if u.hp > 0:
                return self.CLSDSC[u.cls]
        return ""

    def get_hp(self):
        return sum(x.hp for x in self.units if x.hp > 0)

    def playgame(self, retearly = False):
        i = 0
        while self.playround():
            if retearly:
                for u in self.units:
                    if u.hp < 0 and u.cls == "E":
                        return self.CLSDSC["G"], i, self.get_hp()
            i += 1

        return self.get_survivors_cls(), i, self.get_hp()

    def __str__(self):
        self.place_units()
        units = sorted(self.units)
        rows = []
        for y in range(self.height):
            data = ["".join(self.data[y])]

            for u in units:
                if u.y == y and u.hp > 0:
                    data.append("  {}({})".format(u.cls, u.hp))

            rows.append("".join(data))

        self.remove_units()
        return "\n".join(rows)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} <filename>".format(sys.argv[0]), file = sys.stderr)
        exit(-1)

    m = Map()
    try:
        with open(sys.argv[1], "rt") as f:
            m.load(f)
    except:
        raise
        print("Cannot open {} for reading".format(sys.argv[1]), file = sys.stderr)
        exit(-1)

    m1 = copy.deepcopy(m)
    cls, rounds, hp = m1.playgame()
    print(m1)
    print("Combat ends after {} full rounds".format(rounds))
    print("{} win with {} total hit points left".format(cls, hp))
    print("Outcome: {} * {} = {}".format(rounds, hp, rounds * hp))

    print("\nBisecting the attack level for Elves")
    low = 4
    high = 200
    lastm, lastrounds, lasthp = None, None, None
    while low < high:
        mid = (low + high) // 2
        print("Trying with attack {}...".format(mid))
        m1 = copy.deepcopy(m)
        for u in m1.units:
            if u.cls == 'E':
                u.attack = mid

        cls, rounds, hp = m1.playgame(True)
        if cls == m1.CLSDSC["G"]:
            low = mid + 1
        else:
            high = mid

            lastm = m1
            lastrounds = rounds
            lasthp = hp

    if lastm:
        print(lastm)
        print("Minumum attack needed without dead Elves is {}".format(high))
        print("Combat ends after {} full rounds".format(lastrounds))
        print("Elves win with {} total hit points left".format(lasthp))
        print("Outcome: {} * {} = {}".format(lastrounds, lasthp, lastrounds * lasthp))
