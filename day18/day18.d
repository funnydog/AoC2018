import std.algorithm.mutation;
import std.stdio;
import std.conv;

enum
{
	OPEN = '.',
	TREES = '|',
	LUMBER = '#',
}

class Map
{
	char[][] rows;
	char[][] other;

	this(ref File input)
	{
		foreach (line; input.byLine)
		{
			if (line[$-1] == '\n')
			{
				line = line[0..$-1];
			}
			rows ~= line.dup;
		}
		other = std.conv.to!(char[][])(cast (const char[][]) rows);
	}

	this(in Map m)
	{
		rows = std.conv.to!(char[][])(m.rows);
		other = std.conv.to!(char[][])(m.other);
	}

	void adjacent(ulong x, ulong y, out ulong trees, out ulong lumber) const
	{
		static const int[] dx = [0, 1, 1, 1, 0, -1, -1, -1];
		static const int[] dy = [-1, -1, 0, 1, 1, 1, 0, -1];
		trees = lumber = 0;
		for (ulong i = 0; i < 8; i++)
		{
			ulong nx = x + dx[i];
			ulong ny = y + dy[i];
			if (nx < rows[0].length && ny < rows.length)
			{
				switch (rows[ny][nx])
				{
				case TREES: trees++; break;
				case LUMBER: lumber++; break;
				default: break;
				}
			}
		}
	}

	Map next()
	{
		foreach(y, ref row; rows)
		{
			foreach (x, e; row)
			{
				ulong trees, lumber;
				adjacent(x, y, trees, lumber);
				char c;
				switch (e)
				{
				default:
				case OPEN: c = (trees >= 3) ? TREES : OPEN; break;
				case TREES: c = (lumber >= 3) ? LUMBER : TREES; break;
				case LUMBER: c = (lumber >= 1 && trees >= 1) ? LUMBER : OPEN; break;
				}
				other[y][x] = c;
			}
		}
		swap(rows, other);
		return this;
	}

	ulong resources() const
	{
		ulong lumber = 0;
		ulong trees = 0;
		foreach (ref row; rows)
		{
			foreach (v; row)
			{
				switch (v)
				{
				case LUMBER: lumber++; break;
				case TREES: trees++; break;
				default: break;
				}
			}
		}
		return lumber * trees;
	}

	Map dup() const
	{
		return new Map(this);
	}

	override string toString() const
	{
		string v;
		foreach (ref row; rows)
		{
			v ~= row.idup;
			v ~= '\n';
		}
		return v;
	}

	override bool opEquals(Object other) const
	{
		auto that = cast(Map)other;
		if (!that)
		{
			return false;
		}
		return rows == that.rows;
	}
}

int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s <filename>", args[0]);
		return 1;
	}

	Map tortoise;
	try
	{
		File input = File(args[1], "rb");
		tortoise = new Map(input);
		input.close();
	}
	catch (std.exception.ErrnoException)
	{
		stderr.writefln("Cannot open %s", args[1]);
		return 1;
	}

	auto hare = tortoise.dup;
	ulong part1 = 0;
	ulong steps = 0;
	do
	{
		tortoise.next();
		// writeln("\x1b[H");
		// writeln(tortoise);
		hare.next().next();
		steps++;
		if (steps == 10)
		{
			part1 = tortoise.resources();
		}
	} while (tortoise != hare);

	ulong lam = 0;
	hare = tortoise.dup;
	do
	{
		hare.next();
		lam++;
	} while (tortoise != hare);

	ulong left = (1000000000 - steps) % lam;
	while (left-->0)
	{
		tortoise.next();
	}
	ulong part2 = tortoise.resources();

	writeln("Part1: ", part1);
	writeln("Part2: ", part2);
	return 0;
}
