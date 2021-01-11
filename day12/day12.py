#!/usr/bin/env python3

import sys

def parse(txt):
   state = None
   rules = {}
   for line in txt.splitlines():
      lst = line.split(" ")
      if not lst[0]:
         continue
      elif lst[0] == "initial":
         state = lst[2]
      elif lst[1] == "=>" and lst[2] == "#":
         rules[lst[0]] = lst[2]

   return (state.find("#"), state.strip(".")), rules

def step(state, rules):
   pos, plants = state
   plants = "...." + plants + "...."
   new = ""
   for x in range(len(plants)-4):
      new += rules.get(plants[x:x+5], ".")

   pos += new.find("#") - 2
   new = new.strip(".")
   return pos, new

def count_pots(state):
   return sum(i + state[0] for i, v in enumerate(state[1]) if v == "#")

def part1(state, rules):
   for i in range(20):
      state = step(state, rules)

   return count_pots(state)

def part2(state, rules):
   tortoise = step(state, rules)
   hare = step(step(state, rules), rules)
   i = 1
   while tortoise[1] != hare[1]:
      tortoise = step(tortoise, rules)
      hare = step(step(hare, rules), rules)
      i += 1

   cur = count_pots(tortoise)
   incr = count_pots(step(tortoise, rules)) - cur
   return count_pots(tortoise) + (50000000000 - i) * incr

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

   state, rules = parse(txt)
   print("Part1:", part1(state, rules))
   print("Part2:", part2(state, rules))
