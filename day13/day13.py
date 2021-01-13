#!/usr/bin/env python3

import heapq
import sys

UP = 0
RIGHT = 1
DOWN = 2
LEFT = 3

def add(a, b):
    return (a[0]+b[0], a[1]+b[1])

class Cart(object):
    DIRECTIONS = ((0,-1), (1,0), (0,1), (-1,0))

    def __init__(self, pos, d):
        self.pos = pos
        self.d = d
        self.state = 0

    def update(self, mymap):
        self.pos = add(self.pos, self.DIRECTIONS[self.d])
        val = mymap[self.pos[1]][self.pos[0]]
        if val == "+":
            if self.state == 0:
                self.d = (self.d + 3) % 4
                self.state = 1
            elif self.state == 1:
                self.state = 2
            elif self.state == 2:
                self.d = (self.d + 1) % 4
                self.state = 0
        elif val == "\\":
            self.d = (LEFT, DOWN, RIGHT, UP)[self.d]
        elif val == "/":
            self.d = (RIGHT, UP, LEFT, DOWN)[self.d]

    def __lt__(self, other):
        # used by the heap to sort carts by y and x
        return self.pos < other.pos

def simulate(txt):
    # parse the map, the carts and initialize the cartmap
    cartmap = set()
    queue, next_queue = [], []
    directions = {"^": UP, ">": RIGHT, "v": DOWN, "<": LEFT}
    mymap = [list(row) for row in txt.splitlines()]
    for y, row in enumerate(mymap):
        for x, val in enumerate(row):
            if val in directions:
                cart = Cart((x, y), directions[val])
                heapq.heappush(queue, cart)
                mymap[y][x] = val in "^v" and "|" or "-"
                cartmap.add(cart.pos)

    # simulation
    part1 = None
    left = len(queue)
    while left > 1:
        while queue:
            cart = heapq.heappop(queue)
            # skip the cart if it's crashed and therefore its position
            # is not in the cartmap
            if cart.pos not in cartmap:
                continue

            # remove the cart from the cartmap and update it
            cartmap.discard(cart.pos)
            cart.update(mymap)

            # check collisions with other carts
            if cart.pos in cartmap:
                # store the position of the first crash
                if part1 is None:
                    part1 = cart.pos
                # and remove this position from the cartmap
                cartmap.discard(cart.pos)
                left -= 2
            else:
                # the cart didn't crash add it to the next_queue
                heapq.heappush(next_queue, cart)
                # and record its position in the cartmap
                cartmap.add(cart.pos)

        queue, next_queue = next_queue, queue

    # find the first cart not crashed
    part2 = None
    while queue:
        cart = heapq.heappop(queue)
        if cart.pos in cartmap:
            part2 = cart.pos
            break

    return part1, part2

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

    part1, part2 = simulate(txt)
    print("Part1:", part1 and "{},{}".format(*part1) or None)
    print("Part2:", part2 and "{},{}".format(*part2) or None)
