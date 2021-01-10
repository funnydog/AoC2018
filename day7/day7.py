#!/usr/bin/env python3

import heapq
import sys

class Graph(object):
    def __init__(self, txt):
        self.degree = {}
        self.adj = {}
        for line in txt.splitlines():
            a = line.split(" ")
            before, after = a[1], a[7]

            # add the two elements in the graph
            if before not in self.degree:
                self.degree[before] = 0
            self.degree[after] = self.degree.get(after, 0) + 1

            # add the adj
            self.adj[before] = self.adj.get(before, []) + [after]

    def schedule(self, workers):
        # modified Kahn's algorithm with double heaps
        time = 0
        l = []
        s = [name for name, value in self.degree.items() if not value]
        heapq.heapify(s)
        jobs = []
        while True:
            while s and len(jobs) < workers:
                name = heapq.heappop(s)
                heapq.heappush(jobs, (time+60+ord(name)-ord('A')+1, name))

            if not jobs:
                break

            time, name = heapq.heappop(jobs)
            l.append(name)
            for m in self.adj.get(name, []):
                self.degree[m] -= 1
                if not self.degree[m]:
                    heapq.heappush(s, m)

        return "".join(l), time

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} <filename>".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)

    try:
        with open(sys.argv[1], "r") as f:
            txt = f.read().strip()
    except:
        print("Cannot open {} for reading".format(sys.argv[1]), file=sys.stderr)
        sys.exit(1)

    g = Graph(txt)
    print("Part1:", g.schedule(1)[0])
    g = Graph(txt)
    print("Part2:", g.schedule(5)[1])
