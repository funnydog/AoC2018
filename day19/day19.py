#!/usr/bin/env python3

import sys

ADDR = 0
ADDI = 1
MULR = 2
MULI = 3
BANR = 4
BANI = 5
BORR = 6
BORI = 7
SETR = 8
SETI = 9
GTIR = 10
GTRI = 11
GTRR = 12
EQIR = 13
EQRI = 14
EQRR = 15
OPCOUNT = 16

NAMES = {
    "addr": ADDR,
    "addi": ADDI,
    "mulr": MULR,
    "muli": MULI,
    "banr": BANR,
    "bani": BANI,
    "borr": BORR,
    "bori": BORI,
    "setr": SETR,
    "seti": SETI,
    "gtir": GTIR,
    "gtri": GTRI,
    "gtrr": GTRR,
    "eqir": EQIR,
    "eqri": EQRI,
    "eqrr": EQRR,
}

def execute(op, a, b, c, regs):
    if op == ADDR: regs[c] = regs[a] + regs[b]
    elif op == ADDI: regs[c] = regs[a] + b
    elif op == MULR: regs[c] = regs[a] * regs[b]
    elif op == MULI: regs[c] = regs[a] * b
    elif op == BANR: regs[c] = regs[a] & regs[b]
    elif op == BANI: regs[c] = regs[a] & b
    elif op == BORR: regs[c] = regs[a] | regs[b]
    elif op == BORI: regs[c] = regs[a] | b
    elif op == SETR: regs[c] = regs[a]
    elif op == SETI: regs[c] = a
    elif op == GTIR: regs[c] = (a > regs[b]) and 1 or 0
    elif op == GTRI: regs[c] = (regs[a] > b) and 1 or 0
    elif op == GTRR: regs[c] = (regs[a] > regs[b]) and 1 or 0
    elif op == EQIR: regs[c] = (a == regs[b]) and 1 or 0
    elif op == EQRI: regs[c] = (regs[a] == b) and 1 or 0
    elif op == EQRR: regs[c] = (regs[a] == regs[b]) and 1 or 0
    else:
        assert 0, "Unknown operand {}".format(op)

class System(object):
    def __init__(self, txt):
        self.regs = [0] * 6
        self.program = []
        for line in txt.splitlines():
            lst = line.split(" ")
            if lst[0] == "#ip":
                self.ip = int(lst[1])
            else:
                op = NAMES[lst[0]]
                a, b, c = map(int, lst[1:])
                self.program.append((op, a, b, c))

    def reset(self):
        self.regs = [0] * 6

    def run(self, boost = False):
        while 0 <= self.regs[self.ip] < len(self.program):
            if boost and self.regs[self.ip] == 1:
                # in regs[0] there is sum of its divisors
                reg = self.program[self.regs[self.ip]+3][2]
                value = self.regs[reg]
                result = value + 1
                for i in range(2, value//2 + 1):
                    if value % i == 0:
                        result += i

                return result
            else:
                op, a, b, c = self.program[self.regs[self.ip]]
                execute(op, a, b, c, self.regs)
                self.regs[self.ip] += 1

        return self.regs[0]

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

    s = System(txt)
    s.reset()
    print("Part1:", s.run(True))
    s.reset()
    s.regs[0] = 1
    print("Part2:", s.run(True))
