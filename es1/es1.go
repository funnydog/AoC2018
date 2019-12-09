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
	for {
		_, err := fmt.Fscanf(input, "%d\n", &drift)
		if err != nil {
			break
		}
		freq += drift
	}
	fmt.Println("Final frequency", freq)
}
