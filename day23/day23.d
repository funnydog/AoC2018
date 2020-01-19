import std.algorithm.searching;
import std.container.binaryheap;
import std.math;
import std.stdio;

struct bot
{
	long x;
	long y;
	long z;
	long r;

	static ulong manhattan(ref in bot a, ref in bot b)
	{
		return abs(a.x-b.x)+abs(a.y-b.y)+abs(a.z-b.z);
	}
}

static bot[] parse(ref File input)
{
	bot[] lst;
	bot t;
	try
	{
		while (input.readf!"pos=<%s,%s,%s>, r=%s\n"(t.x, t.y, t.z, t.r) == 4)
		{
			lst ~= t;
		}
	}
	catch (std.format.FormatException)
	{
		// do nothing
	}
	return lst;
}

static ulong part1(in bot[] bots)
{
	auto strongest = bots.maxElement!"a.r";
	ulong count = 0;
	foreach(ref b; bots)
	{
		if (bot.manhattan(strongest, b) <= strongest.r)
		{
			count++;
		}
	}
	return count;
}

struct box
{
	long x, y, z;
	long side;
	long distance;
	ulong botcount;

	static long range_distance(long value, long low, long high)
	{
		if (value < low) return low - value;
		if (value > high) return value - high;
		return 0;
	}

	void find_botcount(in bot[] bots)
	{
		botcount = 0;
		foreach(ref b; bots)
		{
			long d = 0;
			d += range_distance(b.x, x, x + side - 1);
			d += range_distance(b.y, y, y + side - 1);
			d += range_distance(b.z, z, z + side - 1);
			if (d <= b.r)
			{
				botcount++;
			}
		}
	}

	int opCmp(ref const box other) const
	{
		if (this.botcount != other.botcount)
		{
			return this.botcount < other.botcount ? 1 : -1;
		}
		if (this.distance != other.distance)
		{
			return this.distance < other.distance ? -1 : 1;
		}
		return this.side < other.side ? -1 : 1;
	}
}

static ulong part2(in bot[] bots)
{
	bot min = bot(long.max, long.max, long.max, 0);
	bot max = bot(long.min, long.min, long.min, 0);
	foreach(ref b; bots)
	{
		if (min.x > b.x - b.r) min.x = b.x - b.r;
		if (min.y > b.y - b.r) min.y = b.y - b.r;
		if (min.z > b.z - b.r) min.z = b.z - b.r;
		if (max.x < b.x + b.r) max.x = b.x + b.r;
		if (max.y < b.y + b.r) max.y = b.y + b.r;
		if (max.z < b.z + b.r) max.z = b.z + b.r;
	}

	box b = box(min.x,  min.y, min.z, 1, abs(min.x)+abs(min.y)+abs(min.z), bots.length);
	while (b.x+b.side < max.x || b.y+b.side < max.y || b.z+b.side < max.z)
	{
		b.side *= 2;
	}

	box[] storage;
	auto q = BinaryHeap!(box[], "a>b")(storage, 0);
	q.insert(b);
	while (!q.empty())
	{
		b = q.front();
		q.popFront();

		if (b.side == 1)
		{
			break;
		}

		auto side = b.side / 2;
		for (long x = b.x; x < b.x + b.side; x += side)
		{
			for (long y = b.y; y < b.y + b.side; y += side)
			{
				for (long z = b.z; z < b.z + b.side; z += side)
				{
					box nb = box(x, y, z, side, abs(x)+abs(y)+abs(z));
					nb.find_botcount(bots);
					q.insert(nb);
				}
			}
		}
	}

	return b.distance;
}

int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s <filename>", args[0]);
		return 1;
	}

	bot[] bots;
	try
	{
		File input = File(args[1], "rb");
		bots = parse(input);
		input.close();
	}
	catch (std.exception.ErrnoException)
	{
		stderr.writefln("Cannot open %s", args[1]);
		return 1;
	}

	writeln("Part1: ", part1(bots));
	writeln("Part2: ", part2(bots));
	return 0;
}
