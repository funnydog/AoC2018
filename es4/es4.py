#!/usr/bin/env python3

from operator import itemgetter, attrgetter

import re

events = []
pattern = re.compile(r"^\[(\d+)-(\d+)-(\d+) (\d+):(\d+)\] (.*)$")
begin = re.compile(r"^Guard #(\d+) begins shift$")
asleep = re.compile(r"^falls asleep$")
wakeup = re.compile(r"^wakes up$")
with open("input.txt", "r") as f:
    for line in f:
        m = pattern.match(line)
        if m is None:
            continue

        etype = 0
        guard_id = None
        t = begin.match(m.group(6))
        if t:
            guard_id = int(t.group(1))
            etype = 0

        t = asleep.match(m.group(6))
        if t:
            etype = 1

        t = wakeup.match(m.group(6))
        if t:
            etype = 2

        events.append((
            int(m.group(1)),
            int(m.group(2)),
            int(m.group(3)),
            int(m.group(4)),
            int(m.group(5)),
            etype,
            guard_id
        ))

# order events by date
events.sort(key = itemgetter(0,1,2,3,4))

class Guard(object):
    def __init__(self, guard_id):
        self.id = guard_id
        self.minutes = [0 for x in range(60)]
        self.total_asleep = 0
        self.max_asleep = -1
        self.max_asleep_minute = -1

    def compute_asleep(self):
        self.total_asleep = 0
        self.max_asleep = -1
        self.max_asleep_minute = -1
        for i in range(len(self.minutes)):
            self.total_asleep += self.minutes[i]
            if self.max_asleep < self.minutes[i]:
                self.max_asleep = self.minutes[i]
                self.max_asleep_minute = i

    def fill(self, minute, end):
        for i in range(minute, end):
            self.minutes[i] += 1

guard_map = {}

guard = None
state = None
minute = None
for e in events:
    if e[5] == 0:
        # Begin shift
        if state == 1 and guard != e[6]:
            guard.fill(minute, 60)

        guard = guard_map.get(e[6])
        if guard is None:
            guard = Guard(e[6])
            guard_map[e[6]] = guard

    elif e[5] == 1:
        # Falls asleep
        minute = e[4]
    elif e[5] == 2:
        # Wakes up
        guard.fill(minute, e[4])

guards = []
for key, guard in guard_map.items():
    guard.compute_asleep()
    guards.append(guard)

guards.sort(key = attrgetter("total_asleep"), reverse = True)
print("Strategy1: {}".format(guards[0].id * guards[0].max_asleep_minute))

guards.sort(key = attrgetter("max_asleep"), reverse = True)
print("Strategy2: {}".format(guards[0].id * guards[0].max_asleep_minute))
