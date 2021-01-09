import std.exception: ErrnoException;
import std.stdio;
import std.file;
import std.ascii: toLower;

ulong reduce(char[] data)
{
	ulong removed = 0;
	ulong i;
	for (i = 0; i < data.length && data[i] == '*'; i++)
	{
		removed++;
	}
	ulong j;
	for (j = i+1; j < data.length && data[j] == '*'; j++)
	{
		removed++;
	}
	while (i < data.length && j < data.length)
	{
		if (data[i] != data[j] && data[i].toLower() == data[j].toLower())
		{
			data[i] = data[j] = '*';
			removed += 2;
			for (j++; j < data.length && data[j] == '*'; j++)
			{
				removed++;
			}
			for (; i > 0 && data[i] == '*'; i--)
			{
			}
		}
		else
		{
			for (i = j; i < data.length && data[i] == '*'; i++)
			{
				removed++;
			}
			for (j = i+1; j < data.length && data[j] == '*'; j++)
			{
				removed++;
			}
		}
	}
	return data.length - removed;
}

ulong part1(char[] data)
{
	return reduce(data);
}

ulong part2(in char[] data)
{
	size_t minlength = data.length;
	char[] copy = new char[data.length];
	for (char c = 'a'; c <= 'z'; c++)
	{
		foreach(i, v; data)
		{
			copy[i] = (v.toLower() == c) ? '*' : v;
		}
		ulong length = reduce(copy);
		if (minlength > length)
		{
			minlength = length;
		}
	}
	return minlength;
}

int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s filename", args[0]);
		return 1;
	}

	char[] data;
	try
	{
		File input = File(args[1], "r");
		data = input.readln().dup;
		data = data[0..$-1]; // chop off the \n
		input.close();
	}
	catch(ErrnoException e)
	{
		stderr.writefln("Cannot open %s", args[1]);
		return 1;
	}

	writeln("Part1: ", part1(data));
	writeln("Part2: ", part2(data));
	return 0;
}
