package main

import (
	"fmt"
	"os"
)

func main() {
	input, err := os.Open("input.txt")
	if err != nil {
		panic(err)
	}
	defer input.Close()

	fabric := [1000][1000]int{}
	for {
		var id, x, y, w, h int

		_, err := fmt.Fscanf(input, "#%d @ %d,%d: %dx%d\n", &id, &x, &y, &w, &h)
		if err != nil {
			break
		}

		for i := y; i < y+h; i++ {
			for j := x; j < x+w; j++ {
				fabric[i][j]++
			}
		}
	}

	sqinches := 0
	for i := 0; i < 1000; i++ {
		for j := 0; j < 1000; j++ {
			if fabric[i][j] > 1 {
				sqinches++
			}
		}
	}
	fmt.Println("Square inches:", sqinches)
}
