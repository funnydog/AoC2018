#!/usr/bin/env python3

with open("input.txt", "r") as f:
    rows = f.read().split("\n")

def distance(a, b):
    if len(a) != len(b):
        return 26

    d = 0
    for i in range(len(a)):
        if a[i] != b[i]:
            d += 1
    return d

w1, w2 = None, None
mind = 26
rowcount = len(rows)-1
for i in range(rowcount-1):
    for j in range(i+1, rowcount):
        d = distance(rows[i], rows[j])
        if d < mind:
            mind = d
            w1 = i
            w2 = j

l = len(rows[i])
common = []
for i in range(l):
    if rows[w1][i] == rows[w2][i]:
        common.append(rows[w1][i])

print("".join(common))
