package main

import (
	"bufio"
	"fmt"
	"os"
)

func main() {
	input, err := os.Open("input.txt")
	if err != nil {
		panic(err)
	}
	defer input.Close()

	twos := 0
	threes := 0
	scanner := bufio.NewScanner(input)
	for scanner.Scan() {
		line := scanner.Text()
		letters := ['z' - 'a' + 1]byte{0}
		for _, byte := range line {
			if byte <= 'z' && byte >= 'a' {
				letters[byte-'a']++
			}
		}

		found2 := false
		found3 := false
		for _, count := range letters {
			if count == 2 {
				found2 = true
			} else if count == 3 {
				found3 = true
			}
		}
		if found2 {
			twos++
		}
		if found3 {
			threes++
		}
	}

	fmt.Println("Checksum:", twos*threes)
}
