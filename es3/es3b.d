import std.stdio;
import std.format;

void main() {
	File input = File("input.txt", "r");

	int[1000][1000] fabric = 0;
	int id, x, y, w, h;
	try {
		while (input.readf(" #%s @ %s,%s: %sx%s", &id, &x, &y, &w, &h)) {
			for (int i = y; i < y + h ; ++i) {
				for (int j = x; j < x + w; ++j) {
					++fabric[i][j];
				}
			}
		}
	} catch  (FormatException e) {
	}

	input.seek(0, 0);
	try {
		while (input.readf(" #%s @ %s,%s: %sx%s", &id, &x, &y, &w, &h)) {
			bool found = true;
			for (int i = y; i < y+h; ++i) {
				for (int j = x; j < x+w; ++j) {
					if (fabric[i][j] > 1) {
						found = false;
					}
				}
			}
			if (found) {
				stdout.writeln("ID ", id);
			}
		}
	} catch (FormatException e) {
	} finally {
		input.close();
	}
}
