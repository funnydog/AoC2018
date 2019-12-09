package main

import (
	"bufio"
	"fmt"
	"os"
)

const (
	NONE = iota
	CLAY
	SAND
	WATER
	MaxInt = int(^uint(0) >> 1)
	MinInt = -MaxInt - 1
)

type Map struct {
	xmin, ymin    int
	xmax, ymax    int
	width, height int

	data [][]int
}

func (m *Map) minmax(x, y int) {
	if m.xmin > x {
		m.xmin = x
	}
	if m.xmax < x {
		m.xmax = x
	}
	if m.ymin > y {
		m.ymin = y
	}
	if m.ymax < y {
		m.ymax = y
	}
}

func (m *Map) Get(x, y int) int {
	if x >= m.xmin && x <= m.xmax && y >= m.ymin && y <= m.ymax {
		return m.data[y-m.ymin][x-m.xmin]
	}
	return NONE
}

func (m *Map) Set(x, y, v int) {
	if x >= m.xmin && x <= m.xmax && y >= m.ymin && y <= m.ymax {
		m.data[y-m.ymin][x-m.xmin] = v
	}
}

func (m *Map) Open(x, y int) bool {
	v := m.Get(x, y)
	return v == NONE || v == SAND
}

func (m *Map) Fill(x, y int) {
	if y > m.ymax || !m.Open(x, y) {
		return
	} else if !m.Open(x, y+1) {
		l := x
		for ; m.Open(l, y) && !m.Open(l, y+1); l-- {
			m.Set(l, y, SAND)
		}

		r := x + 1
		for ; m.Open(r, y) && !m.Open(r, y+1); r++ {
			m.Set(r, y, SAND)
		}

		if m.Open(l, y) || m.Open(r, y) {
			m.Fill(l, y)
			m.Fill(r, y)
		} else {
			l++
			for ; l < r; l++ {
				m.Set(l, y, WATER)
			}
			m.Fill(x, y-1)
		}
	} else if m.Get(x, y) == NONE {
		m.Set(x, y, SAND)
		m.Fill(x, y+1)
	}
}

func (m *Map) Count() (int, int) {
	sand := 0
	water := 0
	for y := 0; y < m.height; y++ {
		for x := 0; x < m.width; x++ {
			if m.data[y][x] == SAND {
				sand++
			} else if m.data[y][x] == WATER {
				water++
			}
		}
	}
	return sand, water
}

func (m *Map) Print() {
	for y := 0; y < m.height; y++ {
		for x := 0; x < m.width; x++ {
			switch m.data[y][x] {
			case NONE:
				fmt.Print(".")
			case CLAY:
				fmt.Print("#")
			case SAND:
				fmt.Print("|")
			case WATER:
				fmt.Print("~")
			}
		}
		fmt.Println()
	}
}

func LoadMap(input *os.File) Map {
	m := Map{xmin: MaxInt, ymin: MaxInt, xmax: MinInt, ymax: MinInt}
	scanner := bufio.NewScanner(input)
	for scanner.Scan() {
		line := scanner.Text()
		var a, b, c int
		_, err := fmt.Sscanf(line, "x=%d, y=%d..%d\n", &a, &b, &c)
		if err == nil {
			m.minmax(a, b)
			m.minmax(a, c)
		}

		_, err = fmt.Sscanf(line, "y=%d, x=%d..%d\n", &a, &b, &c)
		if err == nil {
			m.minmax(b, a)
			m.minmax(c, a)
		}
	}
	m.xmin--
	m.xmax++
	m.width = m.xmax - m.xmin + 1
	m.height = m.ymax - m.ymin + 1

	// build the map
	m.data = make([][]int, m.height)
	for y := 0; y < m.height; y++ {
		m.data[y] = make([]int, m.width)
	}

	input.Seek(0, 0)
	scanner = bufio.NewScanner(input)
	for scanner.Scan() {
		line := scanner.Text()

		var a, b, c int
		_, err := fmt.Sscanf(line, "x=%d, y=%d..%d\n", &a, &b, &c)
		if err == nil {
			for y := b; y <= c; y++ {
				m.Set(a, y, CLAY)
			}
		}

		_, err = fmt.Sscanf(line, "y=%d, x=%d..%d\n", &a, &b, &c)
		if err == nil {
			for x := b; x <= c; x++ {
				m.Set(x, a, CLAY)
			}
		}
	}
	return m
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

	m := LoadMap(input)
	m.Fill(500, 0)
	// m.Print()
	sand, water := m.Count()
	fmt.Printf("sand %d, water %d, sum %d\n", sand, water, sand+water)

	return 0
}
