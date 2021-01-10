import std.math;
import std.stdio;
import std.format;

struct Vec
{
	long x, y;
};

class Light
{
	Vec pos;
	Vec vel;

	this(Vec pos, Vec vel)
	{
		this.pos = pos;
		this.vel = vel;
	}

	Vec at(int time) const
	{
		return Vec(pos.x+vel.x*time, pos.y+vel.y*time);
	}
};

struct Span
{
	long x, y;
	long w, h;
};

Span find_span(in Vec[] pos)
{
	long xmin, ymin, xmax, ymax;
	xmin = ymin = long.max;
	xmax = ymax = long.min;
	foreach (v; pos)
	{
		if (xmin > v.x) xmin = v.x;
		if (xmax < v.x) xmax = v.x;
		if (ymin > v.y) ymin = v.y;
		if (ymax < v.y) ymax = v.y;
	}
	return Span(xmin, ymin, xmax-xmin+1, ymax-ymin+1);
}

ulong find_area(in Light[] lights, int time)
{
	Vec[] pos;
	foreach (ref l; lights)
	{
		pos ~= l.at(time);
	}
	auto s = find_span(pos);
	return cast(ulong)s.w * cast(ulong)s.h;
}

int find_local_minimum(in Light[] lights, int a, int b)
{
	while (abs(a-b) > 2)
	{
		int u = a + (b-a) / 3;
		int v = b - (b-a) / 3;
		if (find_area(lights, u) < find_area(lights, v))
		{
			b = v;
		}
		else
		{
			a = u;
		}
	}

	ulong minarea = find_area(lights, b);
	int time = b;
	foreach(i; a .. b)
	{
		ulong area = find_area(lights, i);
		if (minarea > area)
		{
			minarea = area;
			time = i;
		}
	}
	return time;
}

void render(in Light[] lights, int time)
{
	Vec[] points;
	foreach(ref l; lights)
	{
		points ~= l.at(time);
	}

	auto s = find_span(points);
	for (long y = s.y; y < s.y+s.h; y++)
	{
		for (long x = s.x; x < s.x+s.w; x++)
		{
			char c = ' ';
			foreach(p; points)
			{
				if (p.x == x && p.y == y)
				{
					c = '#';
					break;
				}
			}
			write(c);
		}
		writeln();
	}
}

int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s <filename>", args[0]);
		return 1;
	}

	Light[] lights;
	try
	{
		File input = File(args[1], "r");
		Vec pos, vel;
		while (input.readf(" position=< %s, %s> velocity=< %s, %s>",
				   &pos.x, &pos.y, &vel.x, &vel.y))
		{
			lights ~= new Light(pos, vel);
		}
	}
	catch (std.exception.ErrnoException)
	{
		stderr.writefln("Cannot open %s", args[1]);
		return -1;
	}
	catch (FormatException)
	{
	}

	int time = find_local_minimum(lights, 0, 100000);
	writeln("Part1: (see below)");
	writeln("Part2: ", time);
	render(lights, time);

	return 0;
}
