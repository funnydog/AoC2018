import std.stdio;

struct location
{
	int id;
	int x;
	int y;
}

uint distance(int x, int y)
{
	return (x > 0 ? x : -x ) + (y > 0 ? y : -y);
}

struct region
{
	int x;
	int y;
	int w;
	int h;

	int[][] map;

	this(in location[] locs)
	{
		int xmin, xmax;
		int ymin, ymax;
		xmin = ymin = int.max;
		xmax = ymax = int.min;
		foreach(ref l; locs)
		{
			if (l.x < xmin) xmin = l.x;
			if (l.x > xmax) xmax = l.x;
			if (l.y < ymin) ymin = l.y;
			if (l.y > ymax) ymax = l.y;
		}

		this.x = xmin;
		this.y = ymin;
		this.w = xmax - xmin + 1;
		this.h = ymax - ymin + 1;

		this.map = new int[][](this.h, this.w);
	}

	void partition(in location[] locs)
	{
		for (int y = 0; y < this.h; y++)
		{
			for (int x = 0; x < this.w; x++)
			{
				uint min = uint.max;
				int minlabel = -1;
				foreach (ref l; locs)
				{
					uint d = distance(x + this.x - l.x,
							  y + this.y - l.y);
					if (d == min)
					{
						minlabel = -1;
					}
					else if (d < min)
					{
						min = d;
						minlabel = l.id;
					}
				}
				this.map[y][x] = minlabel;
			}
		}
	}

	uint find_area(in ref location loc) {
		uint area = 0;
		for (int y = 0; y < this.h; y++)
		{
			for (int x = 0; x < this.w; x++)
			{
				if (loc.id == this.map[y][x])
				{
					if (x == 0 || x == this.w - 1 ||
					    y == 0 || y == this.h - 1)
						return 0;
					area++;
				}
			}
		}
		return area;
	}

	uint find_max_area(in location[] locs)
	{
		this.partition(locs);
		uint maxarea = uint.min;
		foreach (ref loc; locs)
		{
			uint area = this.find_area(loc);
			if (area > maxarea)
			{
				maxarea = area;
			}
		}
		return maxarea;
	}

	void compute_distance_sums(in location[] locs)
	{
		for (int y = 0; y < this.h; y++)
		{
			for (int x = 0; x < this.w; x++)
			{
				int sum = 0;
				foreach (ref loc; locs)
				{
					sum += distance(x + this.x - loc.x,
							y + this.y - loc.y);
				}
				this.map[y][x] = sum;
			}
		}
	}

	uint find_area_lt(in location[] locs, int limit)
	{
		this.compute_distance_sums(locs);
		uint area = 0;
		for (int y = 0; y < this.h; y++)
		{
			for (int x = 0; x < this.w; x++)
			{
				if (this.map[y][x] < limit)
				{
					area++;
				}
			}
		}
		return area;
	}
}

int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s filename", args[0]);
		return 1;
	}

	location[] locs;
	try
	{
		File input = File(args[1], "r");
		location loc;
		while (input.readf(" %s, %s", &loc.x, &loc.y)) {
			loc.id = cast(int)locs.length;
			locs ~= loc;
		}
		input.close();
	}
	catch (std.exception.ErrnoException)
	{
		stderr.writefln("Cannot open %s for reading", args[1]);
		return 1;
	}

	region r = region(locs);
	writeln("Part1: ", r.find_max_area(locs));
	writeln("Part2: ", r.find_area_lt(locs, 10000));

	return 0;
}
