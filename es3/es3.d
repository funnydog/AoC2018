import std.stdio;
import std.format;

void main() {
	File input = File("input.txt", "r");

	int[1000][1000] fabric = 0;
	int id, x, y, w, h;
	try {
		while (input.readf("#%s @ %s,%s: %sx%s ", &id, &x, &y, &w, &h)) {
			for (int i = y; i < y+h; ++i) {
				for (int j = x; j < x+w; ++j) {
					++fabric[i][j];
				}
			}
		}
	} catch (FormatException e) {
		// do nothing
	} finally {
		input.close();
	}

	int sqinch = 0;
	for (int i = 0; i < 1000; ++i) {
		for (int j = 0; j < 1000; ++j) {
			if (fabric[i][j] > 1) {
				++sqinch;
			}
		}
	}
	stdout.writeln("Square inches: ", sqinch);
}
