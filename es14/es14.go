package main

import (
	"fmt"
	"os"
	"strconv"
)

func main() {
	rv := mainWithInt()
	if rv != 0 {
		os.Exit(rv)
	}
}

func mainWithInt() int {
	if len(os.Args) < 2 {
		fmt.Fprintf(os.Stderr, "Usage: %s <number>\n", os.Args[0])
		return -1
	}

	number, err := strconv.ParseInt(os.Args[1], 10, 32)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Cannot parse the number %s\n", os.Args[1])
		return -1
	}

	match := []int{}
	for _, c := range os.Args[1] {
		match = append(match, int(c)-'0')
	}
	mc := 0

	var should_terminate = func(digit int) bool {
		if digit == match[mc] {
			mc += 1
		} else if digit == match[0] {
			mc = 1
		} else {
			mc = 0
		}
		return mc == len(match)
	}

	recipes := []int{3, 7}
	e1 := 0
	e2 := 1

	for {
		r := recipes[e1] + recipes[e2]
		if r >= 10 {
			d := r / 10
			recipes = append(recipes, d)
			if should_terminate(d) {
				break
			}

			r -= 10
		}
		recipes = append(recipes, r)
		if should_terminate(r) {
			break
		}

		e1 = (e1 + 1 + recipes[e1]) % len(recipes)
		e2 = (e2 + 1 + recipes[e2]) % len(recipes)
	}

	if len(recipes) >= int(number)+10 {
		fmt.Printf("Solution1: ")
		for i := 0; i < 10; i++ {
			fmt.Printf("%d", recipes[int(number)+i])
		}
		fmt.Println()
	}

	fmt.Println("Solution2:", len(recipes)-len(match))

	return 0
}
