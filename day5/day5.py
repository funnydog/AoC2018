#!/usr/bin/env python3

import sys

def react(data):
    i, j = 0, 1
    length = len(data)
    while i < length and j < length:
        if data[i] != data[j] and data[i].lower() == data[j].lower():
            data[i] = data[j] = "*"
            j += 1
            while i > 0 and data[i] == '*':
                i -= 1
        else:
            i = j
            j = i + 1

    data = [x for x in data if x != "*"]
    return data, len(data)

def part1(txt):
    _, length = react(list(txt))
    return length

def part2(txt):
    data, minlen = react(list(txt))
    letters = set(x.lower() for x in data)
    for letter in letters:
        _, l = react([x for x in data if x.lower() != letter])
        if minlen > l:
            minlen = l
    return minlen

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} <filename>".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)

    try:
        with open(sys.argv[1], "rt") as f:
            txt = f.read().strip()
    except:
        print("Cannot open {}".format(sys.argv[1]), file=sys.stderr)
        sys.exit(1)

    print("Part1:", part1(txt))
    print("Part2:", part2(txt))
