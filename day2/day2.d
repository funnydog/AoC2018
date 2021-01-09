import std.exception: ErrnoException;
import std.stdio;

int part1(in string[] ids)
{
	int twos = 0;
	int threes = 0;
	int['z'-'a'+1] letters;
	foreach(id; ids)
	{
		letters[] = 0;
		foreach(letter; id)
		{
			letters[letter-'a']++;
		}

		bool found2 = false;
		bool found3 = false;
		foreach(count; letters)
		{
			if (count == 2) found2 = true;
			if (count == 3) found3 = true;
		}
		if (found2) twos++;
		if (found3) threes++;
	}
	return twos * threes;
}

string part2(in string[] ids)
{
	foreach(i, a; ids[1 .. $])
	{
		foreach(b; ids[0 .. i])
		{
			ulong last = 0;
			ulong count = 0;
			foreach(j; 0 .. a.length)
			{
				if (a[j] != b[j])
				{
					last = j;
					count++;
				}
			}
			if (count == 1)
			{
				string s = a[0..last] ~ a[last+1 .. $];
				return s;
			}
		}
	}
	return "";
}

string[] load(ref File input)
{
	string[] ids;
	foreach(line; input.byLine)
	{
		ids ~= line.idup();
	}
	return ids;
}

int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s <filename>", args[0]);
		return 1;
	}
	string[] ids;
	try
	{
		File input = File(args[1], "r");
		ids = load(input);
	}
	catch (ErrnoException e)
	{
		stderr.writefln("Cannot open %s", args[1]);
		return 1;
	}

	writefln("Part1: %d", part1(ids));
	writefln("Part2: %s", part2(ids));
	return 0;
}
