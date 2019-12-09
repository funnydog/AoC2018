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
	input.Seek(0, 0)
	for {
		var id, x, y, w, h int
		_, err := fmt.Fscanf(input, "#%d @ %d,%d: %dx%d\n", &id, &x, &y, &w, &h)
		if err != nil {
			break
		}

		found := true
		for i := y; i < y+h; i++ {
			for j := x; j < x+w; j++ {
				if fabric[i][j] > 1 {
					found = false
				}
			}
		}
		if found {
			fmt.Println("ID", id)
		}
	}

}
