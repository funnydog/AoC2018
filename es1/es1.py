#!/usr/bin/env python

with open("input.txt", "r") as f:
    rows = f.read().split("\n")

freq = 0
for drift in rows:
    try:
        freq += int(drift)
    except:
        pass

print(freq)
