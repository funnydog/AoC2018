import std.stdio;
import std.file;
import std.ascii: toLower;

char[] reduce(char[] data) {
	int i = 0;
	while (i < data.length - 1) {
		if (data[i] != data[i+1] && data[i].toLower() == data[i+1].toLower()) {
			data = data[0..i] ~ data[i+2..$];
			if (i > 0) {
				--i;
			}
		} else {
			++i;
		}
	}
	return data;
}

char[] strip(char []data, char c) {
	c = c.toLower();
	int i = 0;
	while (i < data.length) {
		if (data[i].toLower() == c) {
			data = data[0..i] ~ data[i+1..$];
		} else {
			++i;
		}
	}
	return data;
}

int main(string[] args) {
	if (args.length < 2) {
		stderr.writef("Usage: %s filename\n", args[0]);
		return -1;
	}

	File input = File(args[1], "r");
	if (!input.isOpen()) {
		stderr.writef("File %s not found\n", args[1]);
		return -1;
	}

	char[] data = input.readln().dup;
	input.close();

	// chop off the \n
	data = data[0..$-1];

	data = reduce(data);
	writeln("Answer1: ", data.length);

	size_t minlength = data.length;
	char chosen = 0;
	for (char c = 'a'; c <= 'z'; ++c) {
		char[] d = reduce(strip(data, c));
		if (d.length < minlength) {
			minlength = d.length;
			chosen = c;
		}
	}

	writeln("Answer2: ", minlength, " by removing '", chosen, "'");
	return 0;
}
