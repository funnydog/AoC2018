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

	freq := 0
	var drift int
	m := make(map[int]bool)
	for {
		_, err := fmt.Fscanf(input, "%d\n", &drift)
		if err != nil {
			input.Seek(0, 0)
			continue
		}
		freq += drift
		_, found := m[freq]
		if found {
			break
		}
		m[freq] = true
	}
	fmt.Println("Final frequency", freq)
}
