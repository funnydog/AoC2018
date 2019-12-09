import std.stdio;

void main() {
	File input = File("input.txt", "r");

	int freq = 0;
	int drift;
	while (input.readf(" %s", &drift)) {
		freq += drift;
	}
	stdout.writeln("Final frequency ", freq);
}
