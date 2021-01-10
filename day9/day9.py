#!/usr/bin/env python3

import sys

def game(marbles, players):
    marbles += 1
    score = [0] * players
    nxt = [0] * marbles
    prev = [0] * marbles
    cur = 0
    player = 0
    for i in range(1, marbles):
        if i % 23 == 0:
            pos = cur
            for j in range(7):
                pos = prev[pos]

            nxt[prev[pos]] = nxt[pos]
            prev[nxt[pos]] = prev[pos]

            player %= players
            score[player] += pos + i

            cur = nxt[pos]
        else:
            pos1 = nxt[cur]
            pos2 = nxt[pos1]

            nxt[i] = pos2
            prev[i] = pos1
            nxt[pos1] = i
            prev[pos2] = i

            cur = i

        player += 1

    return max(score)

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

    ar = txt.split(" ")
    num_players = int(ar[0])
    last_marble = int(ar[6])

    print("Part1:", game(last_marble, num_players))
    print("Part2:", game(last_marble*100, num_players))
