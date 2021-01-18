#!/usr/bin/env python3

import sys

class Cpu(object):
    def __init__(self, ipreg, program):
        self.regs = [0] * 6
        self.ipreg = ipreg
        self.program = program
        self.divreg = program[26][3]

    def execute(self):
        while 0 <= self.regs[self.ipreg] < len(self.program):
            # NOTE: shortcut to speedup the program
            if self.regs[self.ipreg] == 17:
                self.regs[self.divreg] //= 256
                self.regs[self.ipreg] = 8

            op, a, b, c = self.program[self.regs[self.ipreg]]
            if op == "addr": self.regs[c] = self.regs[a] + self.regs[b]
            elif op == "addi": self.regs[c] = self.regs[a] + b
            elif op == "mulr": self.regs[c] = self.regs[a] * self.regs[b]
            elif op == "muli": self.regs[c] = self.regs[a] * b
            elif op == "banr": self.regs[c] = self.regs[a] & self.regs[b]
            elif op == "bani": self.regs[c] = self.regs[a] & b
            elif op == "borr": self.regs[c] = self.regs[a] | self.regs[b]
            elif op == "bori": self.regs[c] = self.regs[a] | b
            elif op == "setr": self.regs[c] = self.regs[a]
            elif op == "seti": self.regs[c] = a
            elif op == "gtir": self.regs[c] = (a > self.regs[b]) and 1 or 0
            elif op == "gtri": self.regs[c] = (self.regs[a] > b) and 1 or 0
            elif op == "gtrr": self.regs[c] = (self.regs[a] > self.regs[b]) and 1 or 0
            elif op == "eqir": self.regs[c] = (a == self.regs[b]) and 1 or 0
            elif op == "eqri": self.regs[c] = (self.regs[a] == b) and 1 or 0
            elif op == "eqrr": self.regs[c] = (self.regs[a] == self.regs[b]) and 1 or 0
            else: assert 0, "Unknown operand {}".format(op)
            self.regs[self.ipreg] += 1
            if self.regs[self.ipreg] == 30:
                break

        return self

def solve(txt):
    # parse the ipreg and the program
    lines = txt.splitlines()
    assert len(lines) > 0, "Unrecognized program"
    ipreg = int(lines[0].split(" ")[1])
    program = []
    for line in lines[1:]:
        lst = line.split(" ")
        program.append((lst[0], *map(int, lst[1:4])))

    assert len(program) > 30, "Unsuitable program"
    genreg = program[7][3]

    # NOTE: Floyd's algorithm
    tortoise = Cpu(ipreg, program).execute()
    part1 = tortoise.regs[genreg]
    hare = Cpu(ipreg, program).execute().execute()
    while tortoise.regs[genreg] != hare.regs[genreg]:
        tortoise.execute()
        hare.execute().execute()

    # walk the first part of the sequence not in the cycle
    tortoise = Cpu(ipreg, program)
    while tortoise.regs[genreg] != hare.regs[genreg]:
        tortoise.execute()
        hare.execute()

    # walk the cycle
    part2 = None
    hare.regs = tortoise.regs[:]
    hare.execute()
    while tortoise.regs[genreg] != hare.regs[genreg]:
        part2 = hare.regs[genreg]
        hare.execute()

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

    part1, part2 = solve(txt)
    print("Part1:", part1)
    print("Part2:", part2)
