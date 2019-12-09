import std.stdio;

void main() {
	File input = File("input.txt", "r");

	int twos = 0;
	int threes = 0;
	int['z' - 'a' + 1] letters;
	foreach(line; input.byLine) {
		foreach (ref e; letters) {
			e = 0;
		}
		foreach (int b; line) {
			if (b >= 'a' && b <= 'z') {
				++letters[b - 'a'];
			}
		}

		bool found2 = false;
		bool found3 = false;
		foreach(e; letters) {
			if (e == 2) {
				found2 = true;
			} else if (e == 3) {
				found3 = true;
			}
		}
		if (found2) {
			++twos;
		}
		if (found3) {
			++threes;
		}
	}

	stdout.writeln("Checksum: ", twos * threes);
}
