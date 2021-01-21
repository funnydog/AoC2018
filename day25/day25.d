import std.math;
import std.stdio;

struct point
{
	int[4] v;

	ulong parent;
	ulong rank;
}

int distance(ref point a, ref point b)
{
	int r = 0;
	foreach (i; 0..4)
	{
		r += abs(a.v[i] - b.v[i]);
	}
	return r;
}

class Sky
{
	point[] points;

	this(File input)
	{
		try {
			point p;
			while (input.readf(" %d,%d,%d,%d", &p.v[0], &p.v[1], &p.v[2], &p.v[3]) == 4)
			{
				p.parent = points.length;
				p.rank = 0;
				points ~= p;
			}
		}
		catch (std.format.FormatException)
		{
			// do nothing
		}
	}

	ulong find(size_t i)
	{
		while (points[i].parent != i)
		{
			ulong next = points[i].parent;
			points[i].parent = points[next].parent;
			i = next;
		}
		return i;
	}

	void merge(size_t i, size_t j)
	{
		i = find(i);
		j = find(j);

		if (i == j)
		{
			return;
		}

		if (points[i].rank < points[j].rank)
		{
			points[i].parent = j;
		}
		else if (points[i].rank > points[j].rank)
		{
			points[j].parent = i;
		}
		else
		{
			points[i].parent = j;
			points[j].rank++;
		}
	}

	ulong constellation_count()
	{
		foreach (i; 1 .. points.length)
		{
			foreach (j; 0..i)
			{
				if (distance(points[i], points[j]) <= 3)
				{
					merge(i, j);
				}
			}
		}

		ulong count = 0;
		foreach(i, ref p; points)
		{
			if (p.parent == i)
			{
				count++;
			}
		}
		return count;
	}
}

int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s <filename>", args[0]);
		return 1;
	}

	Sky s;
	try
	{
		File input = File(args[1], "rb");
		s = new Sky(input);
		input.close();
	}
	catch (std.exception.ErrnoException)
	{
		stderr.writefln("Cannot open %s", args[1]);
		return 1;
	}

	writeln("Part1: ", s.constellation_count());
	return 0;
}
