package main

import (
	"fmt"
	"os"
)

type Node struct {
	Children []*Node
	Metadata []int
}

func (n *Node) findLicense1() int {
	sum := 0
	for _, m := range n.Metadata {
		sum += m
	}

	for _, c := range n.Children {
		sum += c.findLicense1()
	}

	return sum
}

func (n *Node) findLicense2() int {
	sum := 0
	if len(n.Children) == 0 {
		for _, m := range n.Metadata {
			sum += m
		}
	} else {
		for _, m := range n.Metadata {
			if m > 0 && m <= len(n.Children) {
				sum += n.Children[m-1].findLicense2()
			}
		}
	}
	return sum
}

func buildNode(input *os.File) *Node {
	var ccount, mcount int
	_, err := fmt.Fscanf(input, "%d %d", &ccount, &mcount)
	if err != nil {
		panic(err)
	}

	n := Node{
		Children: make([]*Node, ccount),
		Metadata: make([]int, mcount),
	}

	for i := 0; i < ccount; i++ {
		n.Children[i] = buildNode(input)
	}

	for i := 0; i < mcount; i++ {
		_, err = fmt.Fscanf(input, "%d", &n.Metadata[i])
		if err != nil {
			panic(err)
		}
	}

	return &n
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

	root := buildNode(input)
	if root != nil {
		fmt.Println("License1:", root.findLicense1())
		fmt.Println("License2:", root.findLicense2())
	}

	return 0
}
