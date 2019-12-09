package main

import (
	"fmt"
	"os"
	"strconv"
)

const MinInt = int(-int(^uint(0)>>1) - 1)

type Grid struct {
	data [][]int
}

func (g *Grid) getSum(x, y, s int) int {
	return g.data[y][x] + g.data[y+s][x+s] - g.data[y][x+s] - g.data[y+s][x]
}

func (g *Grid) findLargestBlock(s int) (int, int, int) {
	px := 1
	py := 1
	pp := g.data[py][px]
	for y := 1; y < 301-s; y++ {
		for x := 1; x < 301-s; x++ {
			p := g.getSum(x, y, s)
			if pp < p {
				pp = p
				px = x + 1
				py = y + 1
			}
		}
	}
	return pp, px, py
}

func (g *Grid) findLargestBlockAnySize() (int, int, int, int) {
	var px, py, ps int
	pp := MinInt
	for s := 1; s < 301; s++ {
		p, x, y := g.findLargestBlock(s)
		if pp < p {
			pp = p
			px = x
			py = y
			ps = s
		}
	}
	return pp, px, py, ps
}

func makeGrid(serial int) Grid {
	data := make([][]int, 301)
	data[0] = make([]int, 301)
	for y := 1; y < 301; y++ {
		data[y] = make([]int, 301)
		for x := 1; x < 301; x++ {
			rackId := x + 10
			powerLevel := rackId*y + serial
			powerLevel *= rackId
			powerLevel = ((powerLevel / 100) % 10) - 5

			// summed-are table
			data[y][x] = powerLevel + data[y][x-1] + data[y-1][x] - data[y-1][x-1]
		}
	}
	return Grid{data}
}

func main() {
	rv := mainWithInt()
	if rv != 0 {
		os.Exit(rv)
	}
}

func mainWithInt() int {
	if len(os.Args) < 2 {
		fmt.Fprintf(os.Stderr, "Usage: %s <serial>\n", os.Args[0])
		return -1
	}

	serial, err := strconv.ParseInt(os.Args[1], 10, 32)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Cannot parse the serial %s\n", os.Args[1])
		return -1
	}

	grid := makeGrid(int(serial))

	p, x, y := grid.findLargestBlock(3)
	fmt.Printf("Largest total power: %d at %d,%d\n", p, x, y)

	p, x, y, s := grid.findLargestBlockAnySize()
	fmt.Printf("Largest total power: %d at %d,%d,%d\n", p, x, y, s)

	return 0
}
