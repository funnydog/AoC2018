import std.stdio;
import std.format;

class Grid
{
	int[301][301] data;

	this(int serial)
	{
		for (int y = 1; y <= 300; y++)
		{
			for (int x = 1; x <= 300; x++)
			{
				int rack_id = x + 10;
				int powerlevel = (rack_id * y + serial) * rack_id;
				data[y][x] = (powerlevel/100) % 10 - 5;

				// summed-area table
				data[y][x] += data[y][x-1]
					+ data[y-1][x]
					- data[y-1][x-1];
			}
		}
	}

	int getSum(int x, int y, int s)
	{
		// NOTE: get the sum from (x+1,y+1) -> (x+side,y+side)
		return data[y][x] + data[y+s][x+s] -
			data[y+s][x] - data[y][x+s];
	}

	int findLargestBlock(ref int px, ref int py, int side)
	{
		px = 1;
		py = 1;
		int maxp = int.min;
		foreach (y; 0..300-side)
		{
			foreach (x; 0..300-side)
			{
				int p = getSum(x, y, side);
				if (maxp < p)
				{
					maxp = p;
					px = x + 1;
					py = y + 1;
				}
			}
		}
		return maxp;
	}

	int findLargestSide(ref int px, ref int py, ref int side)
	{
		int maxp = int.min;
		foreach (s; 1..301)
		{
			int x, y, p;
			p = findLargestBlock(x, y, s);
			if (maxp < p)
			{
				maxp = p;
				px = x;
				py = y;
				side = s;
			}
		}
		return maxp;
	}
}

int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s <filename>", args[0]);
		return 1;
	}

	int serial;
	try
	{
		File input = File(args[1], "r");
		input.readf(" %s", &serial);
		input.close();
	}
	catch (std.exception.ErrnoException)
	{
		stderr.writefln("Cannot open %s", args[1]);
		return 1;
	}
	catch (FormatException)
	{
		stderr.writeln("Cannot parse the data");
		return 1;
	}

	Grid grid = new Grid(serial);
	int x, y;
	grid.findLargestBlock(x, y, 3);
	writefln("Part1: %d,%d", x, y);

	int s;
	grid.findLargestSide(x, y, s);
	writefln("Part2: %d,%d,%d", x, y, s);
	return 0;
}
