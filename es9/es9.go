package main

import (
	"fmt"
	"os"
)

type Marble struct {
	Value int64
	Prev  *Marble
	Next  *Marble
}

func (m *Marble) insertAfter(obj *Marble) *Marble {
	obj.Next = m.Next
	obj.Prev = m
	m.Next.Prev = obj
	m.Next = obj
	return obj
}

func (m *Marble) removeAndNext() *Marble {
	m.Next.Prev = m.Prev
	m.Prev.Next = m.Next
	return m.Next
}

func makeMarble(value int64) *Marble {
	m := Marble{Value: value}
	m.Prev = &m
	m.Next = &m
	return &m
}

func main() {
	rv := mainWithInt()
	if rv != 0 {
		os.Exit(rv)
	}
}

func mainWithInt() int {
	if len(os.Args) < 2 {
		fmt.Fprintf(os.Stderr, "Usage: %s <filename>\n", os.Args[0])
		return -1
	}

	input, err := os.Open(os.Args[1])
	if err != nil {
		fmt.Fprintf(os.Stderr, "Cannot open %s for reading\n", os.Args[1])
		return -1
	}
	defer input.Close()

	var numPlayers, lastMarble int64
	_, err = fmt.Fscanf(input, "%d players; last marble is worth %d points",
		&numPlayers, &lastMarble)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Cannot read the numplayers and lastmarble\n")
		return -1
	}

	players := make([]int64, numPlayers)
	marbles := makeMarble(0)

	curMarble := marbles
	curPlayer := int64(0)

	for i := int64(1); i < lastMarble+1; i++ {
		if i%23 != 0 {
			curMarble = curMarble.Next.insertAfter(makeMarble(i))
		} else {
			for j := 0; j < 7; j++ {
				curMarble = curMarble.Prev
			}
			players[curPlayer] += i + curMarble.Value
			curMarble = curMarble.removeAndNext()
		}
		curPlayer = (curPlayer + 1) % numPlayers
	}

	maxScore := int64(0)
	for _, score := range players {
		if maxScore < score {
			maxScore = score
		}
	}

	println("Top score:", maxScore)

	return 0
}
