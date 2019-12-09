#!/usr/bin/env python3

import string

with open("input.txt", "r") as f:
    data = f.read()

data = data[:len(data)-1]

def reduce(data):
    i = 0
    while i < len(data) - 1:
        if data[i] != data[i+1] and data[i].lower() == data[i+1].lower():
            data = data[:i] + data[i+2:]
            if i > 0:
                i -= 1
        else:
            i += 1

    return data

def strip(data, c):
    c = c.lower()
    i = 0
    while i < len(data):
        if data[i].lower() == c:
            data = data[:i] + data[i+1:]
        else:
            i += 1

    return data

data = reduce(data)
print("Answer1:", len(data))

minlen = len(data)
chosen = -1
for x in string.ascii_letters:
    d = reduce(strip(data[:], x))
    if len(d) < minlen:
        minlen = len(d)
        chosen = x

print("Answer2: {} by removing '{}'".format(minlen, chosen))
