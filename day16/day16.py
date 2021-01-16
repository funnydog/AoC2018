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

def parse(txt):
    samples = []
    program = []
    before, after, instr = None, None, None
    emptylines = 0
    state = 0
    for line in txt.splitlines():
        if state == 0:
            if not line:
                if emptylines > 1:
                    state = 1
                else:
                    emptylines += 1
                continue
            elif line.startswith("Before:"):
                before = tuple(map(int, line[9:-1].split(",")))
            elif line.startswith("After:"):
                after = tuple(map(int, line[9:-1].split(",")))
                samples.append((before, instr, after))
            else:
                instr = tuple(map(int, line.split(" ")))
            emptylines = 0
        else:
            if not line:
                continue
            else:
                program.append(tuple(map(int, line.split(" "))))

    return samples, program

def process_samples(samples):
    m = [[0 for x in range(OPCOUNT)] for y in range(OPCOUNT)]
    three_or_more = 0
    for before, instr, after in samples:
        op, a, b, c = instr
        count = 0
        for i in range(16):
            regs = list(before)
            execute(i, a, b, c, regs)
            if tuple(regs) == after:
                count += 1
                m[i][op] = 1
        if count >= 3:
            three_or_more += 1

    return three_or_more, m

def is_col_solved(m, col):
    count, last = 0, -1
    for i, row in enumerate(m):
        if row[col]:
            count += 1
            last = i

    if count != 1:
        return -1

    return last

def solve(m):
    # constraint elimination
    opmap = [None] * OPCOUNT
    for _ in range(OPCOUNT):
        j = -1
        for i in range(OPCOUNT):
            if opmap[i] is None:
                j = is_col_solved(m, i)
                if j >= 0:
                    opmap[i] = j
                    break
        assert j != -1, "Cannot reduce the constraints"
        m[j] = [0] * OPCOUNT

    return opmap

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

    samples, program = parse(txt)

    part1, m = process_samples(samples)
    print("Part1:", part1)

    opmap = solve(m)
    regs = [0] * 4
    for op, a, b, c in program:
        execute(opmap[op], a, b, c, regs)

    print("Part2:", regs[0])
