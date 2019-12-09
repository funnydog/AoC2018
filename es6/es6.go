package main

import (
	"fmt"
	"os"
)

const MaxInt = int(^uint(0) >> 1)
const MinInt = -MaxInt - 1

type Location struct {
	Id int
	X  int
	Y  int
}

type Region struct {
	X int
	Y int
	W int
	H int

	Map [][]int
}

func distance(x0, y0, x1, y1 int) int {
	d := 0
	if x1 > x0 {
		d += x1 - x0
	} else {
		d += x0 - x1
	}

	if y1 > y0 {
		d += y1 - y0
	} else {
		d += y0 - y1
	}
	return d
}

func (r *Region) partition(locs []Location) {
	for y := 0; y < r.H; y++ {
		for x := 0; x < r.W; x++ {
			mind := MaxInt
			label := -1
			for _, l := range locs {
				d := distance(x+r.X, y+r.Y, l.X, l.Y)
				if d == mind {
					label = -1
				} else if d < mind {
					mind = d
					label = l.Id
				}
			}
			r.Map[y][x] = label
		}
	}
}

func (r *Region) FindArea(loc *Location) int {
	area := 0
	for y := 0; y < r.H; y++ {
		for x := 0; x < r.W; x++ {
			if loc.Id == r.Map[y][x] {
				if x == 0 || x == r.W-1 || y == 0 || y == r.H-1 {
					return 0
				}
				area++
			}
		}
	}
	return area
}

func (r *Region) FindLargestArea(locs []Location) int {
	r.partition(locs)
	maxarea := MinInt
	for _, l := range locs {
		a := r.FindArea(&l)
		if maxarea < a {
			maxarea = a
		}
	}
	return maxarea
}

func (r *Region) ComputeDistanceSums(locs []Location) {
	for y := 0; y < r.H; y++ {
		for x := 0; x < r.W; x++ {
			sum := 0
			for _, l := range locs {
				sum += distance(x+r.X, y+r.Y, l.X, l.Y)
			}
			r.Map[y][x] = sum
		}
	}
}

func (r *Region) FindAreaLt(limit int) int {
	area := 0
	for y := 0; y < r.H; y++ {
		for x := 0; x < r.W; x++ {
			if r.Map[y][x] < limit {
				area++
			}
		}
	}
	return area
}

func MakeRegion(locs []Location) Region {
	xmin := MaxInt
	ymin := MaxInt
	xmax := MinInt
	ymax := MinInt
	for _, l := range locs {
		if xmin > l.X {
			xmin = l.X
		}
		if xmax < l.X {
			xmax = l.X
		}
		if ymin > l.Y {
			ymin = l.Y
		}
		if ymax < l.Y {
			ymax = l.Y
		}
	}
	w := xmax - xmin + 1
	h := ymax - ymin + 1
	m := make([][]int, h)
	for i := 0; i < h; i++ {
		m[i] = make([]int, w)
	}
	return Region{xmin, ymin, w, h, m}
}

func main() {
	if len(os.Args) < 2 {
		fmt.Fprintf(os.Stderr, "Usage: %s filename\n", os.Args[0])
		os.Exit(-1)
	}

	input, err := os.Open(os.Args[1])
	if err != nil {
		fmt.Fprintf(os.Stderr, "Cannot open %s for reading\n", os.Args[1])
		os.Exit(-1)
	}

	locs := []Location{}
	var loc Location
	for {
		_, err := fmt.Fscanf(input, "%d, %d\n", &loc.X, &loc.Y)
		if err != nil {
			break
		}
		loc.Id = len(locs)
		locs = append(locs, loc)
	}
	input.Close()

	r := MakeRegion(locs)
	fmt.Printf("Largest area that isn't infinite: %d\n", r.FindLargestArea(locs))

	r.ComputeDistanceSums(locs)
	fmt.Printf("Size of the region with distance sum < 32: %d\n",
		r.FindAreaLt(32))
	fmt.Printf("Size of the region with distance sum < 10000: %d\n",
		r.FindAreaLt(10000))

}
