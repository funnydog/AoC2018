#!/usr/bin/env python

with open("input.txt", "r") as f:
    rows = f.read().split("\n")

def generator():
    while True:
        for drift in rows:
            try:
                yield int(drift)
            except ValueError:
                pass

freq = 0
m = {0: True}
for drift in generator():
    freq += int(drift)
    if freq in m:
        break
    m[freq] = True

print("Final frequency", freq)
