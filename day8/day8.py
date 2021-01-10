#!/usr/bin/env python3

import sys

class Node:
    def __init__(self):
        self.children = []
        self.metadata = []

    def part1(self):
        return sum(self.metadata) + sum(c.part1() for c in self.children)

    def part2(self):
        if not self.children:
            return sum(self.metadata)
        else:
            return sum(self.children[i-1].part2() for i in self.metadata if 0 < i <=len(self.children))

def recursive_build_tree(values, pos = 0):
    nodecount, metadata = values[pos], values[pos+1]
    pos += 2
    node = Node()
    for i in range(nodecount):
        child, pos = recursive_build_tree(values, pos)
        node.children.append(child)

    for i in range(metadata):
        value, pos = values[pos], pos+1
        node.metadata.append(value)

    return node, pos

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} <filename>".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)

    try:
        with open(sys.argv[1], "r") as f:
            txt = f.read().strip()
    except:
        print("Cannot open {}".format(sys.argv[1]), file=sys.stderr)
        sys.exit(1)

    values = list(map(int, txt.split(" ")))
    root, values = recursive_build_tree(values)
    if root:
        print("Part1:", root.part1())
        print("Part2:", root.part2())
