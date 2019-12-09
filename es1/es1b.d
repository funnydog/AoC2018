import std.stdio;

void main() {
	File input = File("input.txt", "r");

	bool[int] map;
	int freq = 0;
	int drift;
	map[freq] = 0;
	while (true) {
		if (input.readf(" %s", &drift) == 0) {
			input.seek(0, 0);
			continue;
		}
		freq += drift;
		if (freq in map) {
			break;
		}
		map[freq] = true;
	}
	stdout.writeln("Final frequency ", freq);
}
