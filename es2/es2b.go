package main

import (
	"bufio"
	"fmt"
	"os"
)

func distance(w1, w2 string) int {
	d := 0
	for i := 0; i < len(w1); i++ {
		if w1[i] != w2[i] {
			d++
		}
	}
	return d
}

func main() {
	input, err := os.Open("input.txt")
	if err != nil {
		panic(err)
	}
	defer input.Close()

	var lines []string
	scanner := bufio.NewScanner(input)
	for scanner.Scan() {
		lines = append(lines, scanner.Text())
	}

	mind := 26
	var w1 string
	var w2 string
	for i := 0; i < len(lines)-1; i++ {
		for j := i + 1; j < len(lines); j++ {
			d := distance(lines[i], lines[j])
			if mind > d {
				mind = d
				w1 = lines[i]
				w2 = lines[j]
			}
		}
	}

	for i := 0; i < len(w1); i++ {
		if w1[i] == w2[i] {
			fmt.Printf("%c", w1[i])
		}
	}
	fmt.Println()
}
