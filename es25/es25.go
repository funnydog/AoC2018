package main

import (
	"fmt"
	"os"
)

type Point struct {
	v [4]int
	p int
	r int
}

func LoadPoint(input *os.File, i int) (Point, error) {
	p := Point{p: i, r: 0}
	_, err := fmt.Fscanf(input, "%d,%d,%d,%d\n", &p.v[0], &p.v[1], &p.v[2], &p.v[3])
	return p, err
}

func Distance(a, b *Point) int {
	d := 0
	for i := 0; i < 4; i++ {
		if a.v[i] > b.v[i] {
			d += a.v[i] - b.v[i]
		} else {
			d += b.v[i] - a.v[i]
		}
	}
	return d
}

type Sky struct {
	points []Point
}

func (s *Sky) find(i int) int {
	for s.points[i].p != i {
		next := s.points[i].p
		s.points[i].p = s.points[next].p
		i = next
	}
	return i
}

func (s *Sky) merge(i, j int) {
	i = s.find(i)
	j = s.find(j)

	if i == j {
	} else if s.points[i].r < s.points[j].r {
		s.points[i].p = j
	} else if s.points[i].r > s.points[j].r {
		s.points[j].p = i
	} else {
		s.points[j].p = i
		s.points[i].r++
	}
}

func (s *Sky) CountConstellations() int {
	for i := 0; i < len(s.points); i++ {
		for j := 0; j < i; j++ {
			if Distance(&s.points[i], &s.points[j]) <= 3 {
				s.merge(i, j)
			}
		}
	}

	count := 0
	for i, p := range s.points {
		if p.p == i {
			count++
		}
	}
	return count
}

func LoadSky(input *os.File) Sky {
	s := Sky{}

	for i := 0; ; i++ {
		p, err := LoadPoint(input, i)
		if err != nil {
			break
		}
		s.points = append(s.points, p)
	}

	return s
}

func main() {
	rv := mainInt()
	if rv != 0 {
		os.Exit(rv)
	}
}

func mainInt() int {
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

	s := LoadSky(input)
	fmt.Println("Constellation count:", s.CountConstellations())

	return 0
}
