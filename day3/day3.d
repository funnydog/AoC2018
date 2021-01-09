import std.algorithm.searching;
import std.exception: ErrnoException;
import std.format: FormatException;
import std.stdio;

class Claim
{
	uint id;
	uint x, y;
	uint w, h;
};

Claim[] load(ref File input)
{
	Claim[] claims;
	Claim c = new Claim;
	try
	{
		while (input.readf("#%s @ %s,%s: %sx%s ", &c.id, &c.x, &c.y, &c.w, &c.h))
		{
			claims ~= c;
			c = new Claim;
		}
	}
	catch (FormatException e)
	{
		// ok do nothing
	}
	return claims;
}

int[][] project(in Claim[] claims)
{
	auto mw = (claims.maxElement!(a => a.x+a.w));
	auto mh = claims.maxElement!(a => a.y+a.h);
	auto width = mw.x + mw.w + 1;
	auto height = mh.y + mh.h + 1;

	auto fabric = new int[][](height, width);
	foreach(ref c; claims)
	{
		foreach(y; c.y .. c.y+c.h)
		{
			foreach(x; c.x .. c.x+c.w)
			{
				fabric[y][x]++;
			}
		}
	}
	return fabric;
}

ulong part1(in int[][] fabric)
{
	ulong count = 0;
	foreach(ref row; fabric)
	{
		foreach(ref c; row)
		{
			if (c > 1)
			{
				count++;
			}
		}
	}
	return count;
}

uint part2(in int[][] fabric, in Claim[] claims)
{
	foreach(ref c; claims)
	{
		bool found = true;
		for (uint y = c.y; found && y < c.y+c.h; y++)
		{
			for(uint x = c.x; found && x < c.x+c.w; x++)
			{
				if (fabric[y][x] != 1)
				{
					found = false;
				}
			}
		}
		if (found)
		{
			return c.id;
		}
	}
	return 0;
}

int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s <filename>", args[0]);
		return 1;
	}

	Claim[] claims;
	try
	{
		File input = File(args[1], "r");
		claims = load(input);
		input.close();
	}
	catch (ErrnoException e)
	{
		stderr.writefln("Cannot open %s", args[1]);
		return 1;
	}

	if (!claims.length)
	{
		stderr.writeln("Cannot parse the data");
		return 1;
	}

	auto fabric = project(claims);
	writefln("Part1: %d", part1(fabric));
	writefln("Part2: %d", part2(fabric, claims));
	return 0;
}
