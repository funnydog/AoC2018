#!/usr/bin/env python3

import heapq
import re
import sys

class Group(object):
    PAT = re.compile(r"\w+|\d+|[;,\(\)]")

    def __init__(self, gtype, txt):
        tokens = self.PAT.findall(txt)
        self.gtype = gtype
        self.units = int(tokens[0])
        self.hitpoints = int(tokens[4])
        self.weakness = []
        self.immunity = []
        i = 7
        if tokens[i] == "(":
            cur = None
            i += 1
            while tokens[i] != ")":
                if tokens[i] == "weak":
                    cur = self.weakness
                elif tokens[i] == "immune":
                    cur = self.immunity
                elif tokens[i] in (";", ",", "to"):
                    pass
                else:
                    cur.append(tokens[i])
                i += 1
            i += 1

        self.attack_damage = int(tokens[i+5])
        self.attack_type = tokens[i+6]
        self.initiative = int(tokens[i+10])
        self.target = None
        self.selected = False
        self.power = self.units * self.attack_damage

    def compute_damage(self, other):
        damage = self.units * self.attack_damage
        if self.attack_type in other.immunity:
            damage = 0
        elif self.attack_type in other.weakness:
            damage *= 2
        return damage

def parse(txt):
    groups = []
    gtype = None
    for line in txt.splitlines():
        if not line:
            pass
        elif line.startswith("Immune"):
            gtype = "immune"
        elif line.startswith("Infection"):
            gtype = "infection"
        else:
            groups.append(Group(gtype, line))

    return groups

def play(txt, boost):
    groups = parse(txt)
    infection = sum(1 for g in groups if g.gtype == "infection")

    # add the boost
    for g in groups:
        if g.gtype == "immune":
            g.attack_damage += boost
            g.power = g.units * g.attack_damage

    deadlock = False
    while not deadlock:
        # target selection
        groups.sort(key = lambda x: (-x.power, -x.initiative))
        for i, a in enumerate(groups):
            if a.units <= 0:
                groups = groups[:i]
                break

            maxdamage = 0
            a.target = None
            for e in groups:
                if e.gtype == a.gtype or e.units <= 0 or e.selected:
                    continue

                damage = a.compute_damage(e)
                if maxdamage < damage:
                    maxdamage = damage
                    a.target = e

            if a.target:
                a.target.selected = True

        # attack
        groups.sort(key = lambda x: -x.initiative)
        deadlock = True
        for a in groups:
            a.selected = False
            if a.units <= 0:
                continue

            if a.target is None:
                continue

            delta = a.compute_damage(a.target) // a.target.hitpoints
            if delta == 0:
                continue

            deadlock = False
            a.target.units -= delta
            a.target.power = a.target.units * a.target.attack_damage
            if a.target.units <= 0 and a.gtype == "immune":
                infection -= 1

    return sum(g.units for g in groups if g.units > 0), infection == 0

def part1(txt):
    return play(txt, 0)[0]

def part2(txt):
    low = 0
    high = 100000
    while low < high:
        mid = (low + high) // 2
        if play(txt, mid)[1]:
            high = mid
        else:
            low = mid + 1

    return play(txt, low)[0]

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} <filename>".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)

    try:
        with open(sys.argv[1], "rt") as f:
            txt = f.read()
    except OSError:
        print("Cannot open {}".format(sys.argv[1]), file=sys.stderr)
        sys.exit(0)

    print("Part1:", part1(txt))
    print("Part2:", part2(txt))
