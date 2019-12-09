import std.stdio;
import std.string;

int distance(ref string w1, ref string w2)
{
	int d = 0;
	for (int i = 0; i < w1.length; i++) {
		if (w1[i] != w2[i])
			++d;
	}
	return d;
}

void main() {
	File input = File("input.txt", "r");

	string[] array;
	char []line;
	while (input.readln(line)) {
		string l = strip(line).idup;
		array ~= l;
	}

	int mind = 26;
	string w1, w2;
	for (int i = 0; i < array.length - 1; ++i) {
		for (int j = i + 1; j < array.length; ++j) {
			int d = distance(array[i], array[j]);
			if (mind > d) {
				mind = d;
				w1 = array[i];
				w2 = array[j];
			}
		}
	}

	for (int i = 0; i < w1.length; i++) {
		if (w1[i] == w2[i]) {
			stdout.write(w1[i]);
		}
	}
	stdout.writeln();
	input.close();
}
