#!/usr/bin/env python3

two = 0
three = 0
with open("input.txt", "r") as f:
    for line in f:
        letters = {}
        for char in line:
            if char in letters:
                letters[char] += 1
            else:
                letters[char] = 1

        found2 = False
        found3 = False
        for letter, count in letters.items():
            if count == 2:
                found2 = True
            elif count == 3:
                found3 = True
        if found2:
            two += 1
        if found3:
            three += 1

    print("Checksum:", two*three)
