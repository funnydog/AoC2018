#!/usr/bin/env python3

import sys

class Timestamp(object):
    def __init__(self, txt):
        date, hour = txt.split(" ")
        self.year, self.month, self.day = map(int, date.split("-"))
        self.hour, self.minute = map(int, hour.split(":"))

    def __lt__(self, other):
        if self.year != other.year:
            return self.year < other.year
        if self.month != other.month:
            return self.month < other.month
        if self.day != other.day:
            return self.day < other.day
        if self.hour != other.hour:
            return self.hour < other.hour
        return self.minute < other.minute

class Guard(object):
    def __init__(self, id):
        self.id = id
        self.minutes = [0] * 60
        self.total_asleep = 0
        self.max_asleep = 0
        self.max_asleep_minute = 0

    def increase(self, start, end):
        for i in range(start, end):
            self.minutes[i] += 1

    def compute_asleep(self):
        self.total_asleep = 0
        self.max_asleep = 0
        self.max_asleep_minute = 0
        for i, count in enumerate(self.minutes):
            self.total_asleep += count
            if self.max_asleep < count:
                self.max_asleep = count
                self.max_asleep_minute = i

def parse_guards(txt):
    events = []
    for line in txt.splitlines():
        pos = line.find("]")
        if pos < 0:
            continue
        ts = Timestamp(line[1:pos])
        lst = line[pos+2:].split(" ")
        if lst[0] == "Guard":
            events.append((ts, int(lst[1][1:]), "on"))
        elif lst[0] == "falls":
            events.append((ts, None, "off"))
        elif lst[0] == "wakes":
            events.append((ts, None, "on"))

    events.sort(key=lambda x: x[0])

    guards = {}
    last_ts = None
    current_guard = None
    for ts, guard, event in events:
        if not guard is None:
            current_guard = guards.get(guard)
            if not current_guard:
                current_guard = Guard(guard)
                guards[guard] = current_guard
            last_ts = None
        elif event == "on" and last_ts:
            current_guard.increase(last_ts.minute, ts.minute)
            last_ts = None
        elif event == "off":
            last_ts = ts

    guards = list(guards.values())
    for g in guards:
        g.compute_asleep()
    return guards

def part1(guards):
    guards.sort(key = lambda x: -x.total_asleep)
    return guards[0].id * guards[0].max_asleep_minute

def part2(guards):
    guards.sort(key = lambda x: -x.max_asleep)
    return guards[0].id * guards[0].max_asleep_minute

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

    guards = parse_guards(txt)
    print("Part1:", part1(guards))
    print("Part2:", part2(guards))
