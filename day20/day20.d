import std.stdio;

struct Pos
{
	int x;
	int y;
}

class Map
{
	char[Pos] data;
	int x, y;

	this(char[] expr)
	{
		x = y = 0;
		data.clear();
		walk(expr);
	}

	void walk(ref char[] expr)
	{
		int ox = x;
		int oy = y;
		while (expr.length)
		{
			auto v = expr[0];
			expr = expr[1 .. $];
			if (v == ')' || v == '$')
			{
				break;
			}
			else if (v == '^')
			{
				x = y = 0;
				data[Pos(x, y)] = 'X';
			}
			else if (v == '|')
			{
				x = ox;
				y = oy;
			}
			else if (v == '(')
			{
				walk(expr);
			}
			else
			{
				int dx = 0, dy = 0;
				switch (v)
				{
				case 'N': dy = -1; break;
				case 'W': dx = -1; break;
				case 'S': dy = 1; break;
				case 'E': dx = 1; break;
				default: assert(0, "Unknown character " ~ v);
				}
				// place the door
				x += dx;
				y += dy;
				data[Pos(x, y)] = dx == 0 ? '-' : '|';
				// place the room
				x += dx;
				y += dy;
				data[Pos(x, y)] = '.';
			}
		}
	}

	override string toString() const
	{
		int xmin, xmax, ymin, ymax;
		xmin = ymin = int.max;
		xmax = ymax = int.min;
		foreach (k,_; data)
		{
			if (xmin > k.x) xmin = k.x;
			if (xmax < k.x) xmax = k.x;
			if (ymin > k.y) ymin = k.y;
			if (ymax < k.y) ymax = k.y;
		}

		string s;
		Pos p;
		for (int y = ymin - 1; y <= ymax + 1; y++)
		{
			p.y = y;
			for (int x = xmin - 1; x <= xmax + 1; x++)
			{
				p.x = x;
				if (p in data)
				{
					s ~= data[p];
				}
				else
				{
					s ~= '#';
				}
			}
			s ~= '\n';
		}
		return s;
	}

	void bfs(Pos p, out ulong part1, out ulong part2)
	{
		static const int[] dx = [0, 1, 0, -1];
		static const int[] dy = [-1, 0, 1, 0];
		ulong[Pos] distance;
		Pos[] queue;

		distance[p] = 0;
		queue ~= p;
		while (queue.length)
		{
			p = queue[0];
			queue = queue[1 .. $];
			for (int i = 0; i < 4; i++)
			{
				Pos np = Pos(p.x+dx[i], p.y+dy[i]);
				if (!(np in data) || (np in distance))
				{
					continue;
				}

				distance[np] = distance[p]+1;
				queue ~= np;
			}
		}

		part1 = part2 = 0;
		foreach (k, v; distance)
		{
			if (part1 < v)
			{
				part1 = v;
			}
			if (v >= 2000 && data[k] == '.')
			{
				part2++;
			}
		}
		part1 /= 2;
	}
}

int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s <filename>", args[0]);
		return 1;
	}

	char[] txt;
	try
	{
		File input = File(args[1], "rb");
		input.readln(txt);
		input.close();
	}
	catch (std.exception.ErrnoException)
	{
		stderr.writefln("Cannot open %s", args[1]);
		return 1;
	}

	Map m = new Map(txt);
	//writeln(m);
	ulong part1, part2;
	m.bfs(Pos(0, 0), part1, part2);

	writeln("Part1: ", part1);
	writeln("Part2: ", part2);
	return 0;
}
