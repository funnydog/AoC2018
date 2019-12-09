#!/usr/bin/env python3

import sys

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} number".format(sys.argv[0]), file = sys.stderr)
        exit(-1)

    try:
        number = int(sys.argv[1])
    except:
        print("Cannot parse the number {}".format(sys.argv[1]), file = sys.stderr)
        exit(-1)

    recipes = [3, 7]
    e1 = 0
    e2 = 1

    match = [int(x) for x in sys.argv[1]]
    mc = 0

    def should_end(digit):
        global mc
        if digit == match[mc]:
            mc += 1
        elif digit == match[0]:
            mc = 1
        else:
            mc = 0

        return mc == len(match)

    it = 0
    while True:
        it += 1
        recipe = recipes[e1] + recipes[e2]
        if recipe >= 10:
            digit = recipe // 10
            recipes.append(digit)
            if should_end(digit):
                break

        digit = recipe % 10
        recipes.append(digit)
        if should_end(digit):
            break

        e1 = (e1 + recipes[e1] + 1) % len(recipes)
        e2 = (e2 + recipes[e2] + 1) % len(recipes)

    if len(recipes) >= number + 10:
        out = []
        for i in range(10):
            out.append(str(recipes[number+i]))

        print("Sequence: {}".format("".join(out)))

    if mc == len(match):
        print("Recipes on left: {}".format(len(recipes) - mc))

    print("Number of iterations: {}".format(it))
