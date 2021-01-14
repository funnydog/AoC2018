import std.ascii;
import std.conv;
import std.stdio;
import std.string;

static void make_recipes(string txt, out ulong part1, out ulong part2)
{
	char[] values;
	foreach (v; txt)
	{
		if (v.isDigit)
		{
			values ~= v - '0';
		}
	}

	char[] seq;
	seq ~= 3;
	seq ~= 7;
	ulong e1 = 0;
	ulong e2 = 1;
	while (true)
	{
		auto v = seq[e1] + seq[e2];
		auto q = v / 10;
		auto r = v % 10;
		if (q)
		{
			seq ~= q;
			if (seq.length >= values.length && seq[$-values.length .. $] == values)
			{
				break;
			}

		}
		seq ~= r;
		if (seq.length >= values.length && seq[$-values.length .. $] == values)
		{
			break;
		}

		e1 = (e1 + seq[e1] + 1) % seq.length;
		e2 = (e2 + seq[e2] + 1) % seq.length;
	}

	part1 = 0;
	ulong start = to!ulong(txt);
	if (seq.length >= start+10)
	{
		foreach(v; seq[start .. start+10])
		{
			part1 = part1 * 10 + v;
		}
	}
	part2 = (seq.length >= txt.length) ? (seq.length - txt.length) : 0;
}

int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s <filename>", args[0]);
		return 1;
	}

	string number;
	try
	{
		File input = File(args[1], "r");
		number = strip(input.readln());
		input.close();
	}
	catch (std.exception.ErrnoException)
	{
		stderr.writefln("Cannot open %s", args[1]);
		return 1;
	}

	ulong part1, part2;
	make_recipes(number, part1, part2);
	writefln("Part1: %010d", part1);
	writefln("Part2: %d", part2);
	return 0;
}
