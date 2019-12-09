package main

import (
	"fmt"
	"io/ioutil"
	"os"
)

func lower(a byte) byte {
	return a | 32
}

func reduce(data []byte) []byte {
	i := 0
	for i < len(data)-1 {
		if data[i] != data[i+1] && lower(data[i]) == lower(data[i+1]) {
			data = append(data[:i], data[i+2:]...)
			if i > 0 {
				i--
			}
		} else {
			i++
		}
	}
	return data
}

func strip(data []byte, c byte) []byte {
	c = lower(c)
	i := 0
	for i < len(data) {
		if lower(data[i]) == c {
			data = append(data[:i], data[i+1:]...)
		} else {
			i++
		}
	}
	return data
}

func main() {
	if len(os.Args) < 2 {
		fmt.Fprintf(os.Stderr, "Usage: %s filename\n", os.Args[0])
		os.Exit(-1)
	}
	data, err := ioutil.ReadFile(os.Args[1])
	if err != nil {
		fmt.Fprintf(os.Stderr, "File %s not found\n", os.Args[1])
		os.Exit(-1)
	}

	// chop the last element (\n)
	data = data[:len(data)-1]

	data = reduce(data)
	fmt.Printf("Answer1: %d\n", len(data))

	minlen := len(data)
	var chosen byte
	for c := byte('a'); c <= byte('z'); c++ {
		dup := make([]byte, len(data))
		copy(dup, data)
		d := reduce(strip(dup, c))
		if len(d) < minlen {
			minlen = len(d)
			chosen = c
		}
	}
	fmt.Printf("Answer2: %d by removing '%c'\n", minlen, chosen)
}
