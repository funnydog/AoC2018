import std.exception: ErrnoException;
import std.stdio;

int[] load_numbers(ref File input)
{
	int []numbers;
	int number;
	while (input.readf(" %s", &number))
	{
		numbers ~= number;
	}
	return numbers;
}

int part1(in int[] numbers)
{
	int sum = 0;
	foreach (v; numbers)
	{
		sum += v;
	}
	return sum;
}

int part2(in int[] numbers)
{
	bool[int] seen;
	int freq = 0;
	while (true)
	{
		foreach(v; numbers)
		{
			seen[freq] = true;
			freq += v;
			if (seen.get(freq, false))
			{
				return freq;
			}
		}
	}
}

int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s <filename>", args[0]);
		return 1;
	}
	int[] numbers;
	try
	{
		File input = File(args[1], "r");
		numbers = load_numbers(input);
		input.close();
	}
	catch (ErrnoException e)
	{
		stderr.writefln("Cannot open %s", args[1]);
		return 1;
	}

	writefln("Part1: %d", part1(numbers));
	writefln("Part2: %d", part2(numbers));
	return 0;
}
