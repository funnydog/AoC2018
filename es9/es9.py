#!/usr/bin/env python3

import os
import sys

class Marble(object):
    def __init__(self, value):
        self.value = value
        self.prev = self
        self.next = self

    def insert_after(self, marble):
        marble.next = self.next
        marble.prev = self
        self.next.prev = marble
        self.next = marble
        return marble

    def remove_and_next(self):
        self.next.prev = self.prev
        self.prev.next = self.next
        return self.next

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} <filename>".format(sys.argv[0]), file = sys.stderr)
        exit(-1)

    try:
        with open(sys.argv[1], "r") as f:
            ar = f.read().split(" ")
    except:
        print("Cannot open {} for reading".format(sys.argv[1]), file = sys.stderr)
        exit(-1)

    num_players = int(ar[0])
    last_marble = int(ar[6])

    players = [0 for x in range(num_players)]
    marbles = Marble(0)

    cur_marble = marbles
    cur_player = 0
    for i in range(1, last_marble+1):
        if i % 23 != 0:
            cur_marble = cur_marble.next.insert_after(Marble(i))
        else:
            for x in range(7):
                cur_marble = cur_marble.prev
            players[cur_player] += i + cur_marble.value
            cur_marble = cur_marble.remove_and_next()

        cur_player = (cur_player + 1) % num_players

    top_score = 0
    for score in players:
        if top_score < score:
            top_score = score

    print("Top score: {}".format(top_score))
