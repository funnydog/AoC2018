import std.stdio;
import std.format;

struct Pos
{
	int x;
	int y;

	Pos opBinary(string op)(in Pos rhs) const
	{
		static if (op == "+") return Pos(x+rhs.x, y+rhs.y);
		else static assert(0, "Operator " ~ op ~ " not implemented");
	}
}

static const Pos UP = Pos(0, -1);
static const Pos RIGHT = Pos(1, 0);
static const Pos DOWN = Pos(0, 1);
static const Pos LEFT = Pos(-1, 0);

class Map
{
	int ymin, ymax;
	char[Pos] map;

	void set(in Pos p, char value)
	{
		map[p] = value;
	}

	bool open(in Pos p) const
	{
		return p in map ? map[p] == '|' : true;
	}

	void fill(in Pos p)
	{
		if (p.y > ymax || !open(p))
		{
			return;
		}
		if (!this.open(p + DOWN))
		{
			Pos left;
			for (left = p;
			     open(left) && !open(left + DOWN);
			     left = left + LEFT)
			{
				map[left] = '|';
			}

			Pos right;
			for (right = p + RIGHT;
			     open(right) && !open(right + DOWN);
			     right = right + RIGHT)
			{
				map[right] = '|';
			}

			if (open(left) || open(right))
			{
				fill(left);
				fill(right);
			}
			else
			{
				left = left + RIGHT;
				while (left != right)
				{
					map[left] = '~';
					left = left + RIGHT;
				}
				fill(p + UP);
			}
		}
		else if (!(p in map))
		{
			map[p] = '|';
			fill(p + DOWN);
		}
	}

	void count(out ulong sand, out ulong water)
	{
		sand = water = 0;
		foreach (k, v; map)
		{
			if (k.y < ymin)
			{
			}
			else if (v == '|')
			{
				sand++;
			}
			else if (v == '~')
			{
				water++;
			}
		}
	}

	override string toString() const
	{
		int xmin = int.max;
		int xmax = int.min;
		foreach(k, _; map)
		{
			if (xmin > k.x) xmin = k.x;
			if (xmax < k.x) xmax = k.x;
		}
		string value;
		for (int y = 0; y < this.ymax+1; y++)
		{
			for (int x = xmin; x < xmax+1; x++)
			{
				auto p = Pos(x, y);
				value ~= (p in map) ? map[p] : '.';
			}
			value ~= '\n';
		}
		return value;
	}

	this(ref File input)
	{
		foreach (line; input.byLine)
		{
			if (line[0] == 'x')
			{
				int x, y1, y2;
				line.formattedRead("x=%s, y=%s..%s", x, y1, y2);
				foreach (y; y1 .. y2+1)
				{
					map[Pos(x, y)] = '#';
				}
			}
			else if(line[0] == 'y')
			{
				int y, x1, x2;
				line.formattedRead("y=%s, x=%s..%s", y, x1, x2);
				foreach(x; x1 .. x2 + 1)
				{
					map[Pos(x, y)] = '#';
				}
			}
		}
		ymin = int.max;
		ymax = int.min;
		foreach(p, _; map)
		{
			if (ymin > p.y) ymin = p.y;
			if (ymax < p.y) ymax = p.y;
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

	Map m;
	try
	{
		File input = File(args[1], "rb");
		m = new Map(input);
		input.close();
	}
	catch (std.exception.ErrnoException)
	{
		stderr.writefln("Cannot open file %s", args[1]);
		return 1;
	}

	// writeln(m);
	m.fill(Pos(500,0));
	// writeln(m);

	ulong sand, water;
	m.count(sand, water);
	writeln("Part1: ", sand+water);
	writeln("Part2: ", water);
	return 0;
}
