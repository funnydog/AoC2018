#!/usr/bin/env python3

import sys

class Node:
    def __init__(self, ccount, mcount):
        self.children = [None for x in range(ccount)]
        self.metadata = [0 for x in range(mcount)]

    def find_license1(self):
        s = 0
        for x in self.metadata:
            s += x

        for c in self.children:
            s += c.find_license1()

        return s

    def find_license2(self):
        if len(self.children) == 0:
            return sum(self.metadata)
        else:
            s = 0
            for i in self.metadata:
                if i > 0 and i <= len(self.children):
                    s += self.children[i-1].find_license2()
            return s


def build_tree(values):
    node = Node(values[0], values[1])
    values = values[2:]

    for i in range(len(node.children)):
        node.children[i], values = build_tree(values)

    for i in range(len(node.metadata)):
        node.metadata[i] = values[0]
        values = values[1:]

    return node, values

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} <filename>".format(sys.argv[0]), file = sys.stderr)
        exit(-1)

    try:
        with open(sys.argv[1], "r") as f:
            values = [int(x) for x in f.read().split(" ")]
            root, values = build_tree(values)
            if root:
                print("License1:", root.find_license1())
                print("License2:", root.find_license2())
    except:
        print("Cannot open {} for reading".format(sys.argv[1]))
        raise
