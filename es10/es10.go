package main

import (
	"fmt"
	"os"
)

const MaxInt64 = int64(^uint64(0) >> 1)
const MinInt64 = -MaxInt64 - 1

type Light struct {
	X, Y   int64
	Vx, Vy int64
}

func (l *Light) stepForward() {
	l.X += l.Vx
	l.Y += l.Vy
}

func (l *Light) stepBackwards() {
	l.X -= l.Vx
	l.Y -= l.Vy
}

type Extents struct {
	Xmin, Ymin int64
	Xmax, Ymax int64
}

func findArea(lights []*Light) (Extents, int64) {
	e := Extents{MaxInt64, MaxInt64, MinInt64, MinInt64}
	for _, l := range lights {
		if e.Xmin > l.X {
			e.Xmin = l.X
		}
		if e.Xmax < l.X {
			e.Xmax = l.X
		}
		if e.Ymin > l.Y {
			e.Ymin = l.Y
		}
		if e.Ymax < l.Y {
			e.Ymax = l.Y
		}
	}
	return e, (e.Xmax - e.Xmin + 1) * (e.Ymax - e.Ymin + 1)
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

	lights := []*Light{}
	for {
		var x, y, vx, vy int64
		_, err = fmt.Fscanf(input, "position=<%d,%d> velocity=<%d,%d>\n",
			&x, &y, &vx, &vy)
		if err != nil {
			break
		}
		lights = append(lights, &Light{x, y, vx, vy})
	}

	fmt.Printf("Read %d light points\n", len(lights))

	minArea := MaxInt64
	minSec := 0
	minExt := Extents{}
	curSec := 0
	for ; ; curSec++ {
		ext, area := findArea(lights)
		if minArea > area {
			minArea = area
			minSec = curSec
			minExt = ext
		} else if minArea < area {
			break
		}
		for _, l := range lights {
			l.stepForward()
		}
	}

	fmt.Printf("Minimum reached after %d seconds, area %d\n", minSec, minArea)

	for ; curSec > minSec; curSec-- {
		for _, l := range lights {
			l.stepBackwards()
		}
	}

	fmt.Printf("Map size %dx%d starting at(%d,%d)\n",
		minExt.Xmax-minExt.Xmin+1,
		minExt.Ymax-minExt.Ymin+1,
		minExt.Xmin, minExt.Ymin)

	for y := minExt.Ymin; y <= minExt.Ymax; y++ {
		for x := minExt.Xmin; x <= minExt.Xmax; x++ {
			found := false
			for _, l := range lights {
				if l.X == x && l.Y == y {
					fmt.Printf("#")
					found = true
					break
				}
			}
			if !found {
				fmt.Printf(" ")
			}
		}
		fmt.Println()
	}

	return 0
}
