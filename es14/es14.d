import std.stdio;
import std.conv;

int main(string[] args) {
	if (args.length < 2) {
		stderr.writef("Usage: %s <number>\n", args[0]);
		return -1;
	}

	int number;
	try {
		number = to!int(args[1]);
	} catch (std.conv.ConvException) {
		stderr.writef("Cannot parse the number %s\n", args[1]);
		return -1;
	}

	int[] match;
	for (size_t i = 0; i < args[1].length; i++) {
		match ~= args[1][i] - '0';
	}
	int mc = 0;

	bool should_end(int digit) {
		if (digit == match[mc]) {
			++mc;
		} else if (digit == match[0]) {
			mc = 1;
		} else {
			mc = 0;
		}
		return (mc == match.length);
	}

	int[] recipes = [3, 7];
	int e1 = 0;
	int e2 = 1;

	for (int i = 0; ; i++) {
		int r = recipes[e1] + recipes[e2];
		if (r >= 10) {
			int d = r / 10;
			recipes ~= d;
			if (should_end(d))
				break;

			r -= 10;
		}
		recipes ~= r;
		if (should_end(r))
			break;

		e1 = (e1 + 1 + recipes[e1]) % cast(int)recipes.length;
		e2 = (e2 + 1 + recipes[e2]) % cast(int)recipes.length;
	}

	if (recipes.length >= number + 10) {
		writef("Solution1: ");
		for (int i = 0; i < 10; i++) {
			writef("%d", recipes[number+i]);
		}
		writef("\n");
	}

	writef("Solution2: %d\n", recipes.length - match.length);

	return 0;
}
