import std.stdio;
import std.format;

class Node
{
	Node[] children;
	int[] metadata;

	int part1()
	{
		int sum = 0;
		foreach(m; this.metadata)
		{
			sum += m;
		}
		foreach(c; this.children)
		{
			sum += c.part1();
		}
		return sum;
	}

	int part2()
	{
		if (this.children.length == 0)
		{
			int sum = 0;
			foreach(m; this.metadata)
			{
				sum += m;
			}
			return sum;
		}
		else
		{
			int sum = 0;
			foreach(m; this.metadata)
			{
				if (0 < m && m <= this.children.length)
				{
					sum += this.children[m-1].part2();
				}
			}
			return sum;
		}
	}
}

Node recursive_build_tree(ref File input)
{
	Node n = new Node;
	ulong ccount, mcount;
	try
	{
		input.readf(" %s %s", &ccount, &mcount);
	}
	catch (FormatException e)
	{
		return n;
	}
	while(ccount-->0)
	{
		n.children ~= recursive_build_tree(input);
	}
	while (mcount-->0)
	{
		int m;
		input.readf(" %s", &m);
		n.metadata ~=  m;
	}
	return n;
}

int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s <filename>", args[0]);
		return 1;
	}

	Node root;
	try
	{
		File input = File(args[1], "r");
		root = recursive_build_tree(input);
		input.close();
	}
	catch (std.exception.ErrnoException e)
	{
		stderr.writefln("Cannot open %s", args[1]);
		return 1;
	}

	writeln("Part1: ", root.part1());
	writeln("Part2: ", root.part2());
	return 0;
}
