import std.container.binaryheap;
import std.math;
import std.stdio;

enum Tools
{
	NONE,
	TORCH,
	GEAR,
}

struct state
{
	int x;
	int y;
	int tool;
	int priority;

	int opCmp(const state other)
	{
		return this.priority - other.priority;
	}

	bool opEquals(ref const state other) const
	{
		return this.x == other.x &&
			this.y == other.y &&
			this.tool == other.tool;
	}

	size_t toHash() const nothrow @safe
	{
		return (x << 10) ^ (y << 2)  ^ tool;
	}
}

static ulong[][] erosion;

static void compute_erosion(ulong salt, in state target, ulong x1, ulong y1)
{
	// old rows
	foreach (y; 0..erosion.length)
	{
		foreach (x; erosion[y].length .. x1+1)
		{
			ulong v;
			if (x == target.x && y == target.y)
			{
				v = salt;
			}
			else if (x == 0 || y == 0)
			{
				v = salt + x * 16807 + y * 48271;
			}
			else
			{
				v = salt + erosion[y][x-1] * erosion[y-1][x];
			}
			erosion[y] ~= v % 20183;
		}
	}

	// new rows
	foreach (y; erosion.length .. y1+1)
	{
		ulong[] row;
		foreach (x; 0..x1+1)
		{
			ulong v;
			if (x == target.x && y == target.y)
			{
				v = salt;
			}
			else if (x == 0 || y == 0)
			{
				v = salt + x * 16807 + y * 48271;
			}
			else
			{
				v = salt + row[x-1] * erosion[y-1][x];
			}
			row ~= v % 20183;
		}
		erosion ~= row;
	}
}

static ulong rect_risk(in ulong salt, in state target)
{
	compute_erosion(salt, target, target.x+1, target.y+1);
	ulong r = 0;
	foreach (y; 0..target.y+1)
	{
		foreach(x; 0..target.x+1)
		{
			r += erosion[y][x] % 3;
		}
	}
	return r;
}

static ulong get_type(in ulong salt, in state target, ulong x, ulong y)
{
	if (y >= erosion.length || x >= erosion[y].length)
	{
		compute_erosion(salt, target, x * 2, y * 2);
	}
	return erosion[y][x] % 3;
}

static ulong astar(in ulong salt, in state target)
{
	static const int[] dx = [0, 0, 1, 0, -1];
	static const int[] dy = [0, -1, 0, 1, 0];

	state start = state(0, 0, Tools.TORCH, 0);

	ulong[state] distance;
	distance[start] = 0;

	state[] values;
	auto queue = BinaryHeap!(state[], "a>b")(values, 0);
	queue.insert(start);
	while (!queue.empty())
	{
		auto cur = queue.front();
		queue.removeFront();
		if (cur == target)
		{
			return distance[cur];
		}

		auto curtype = get_type(salt, target, cur.x, cur.y);
		foreach (i; 0..5)
		{
			state nstate = state(cur.x+dx[i], cur.y+dy[i]);
			if (nstate.x < 0 || nstate.y < 0)
			{
				continue;
			}

			auto type = get_type(salt, target, nstate.x, nstate.y);
			foreach (tool; 0..3)
			{
				nstate.tool = tool;
				if (cur == nstate || type == tool || curtype == tool)
				{
					continue;
				}

				auto dist = distance[cur];
				if (cur.x != nstate.x || cur.y != nstate.y)
				{
					dist++;
				}
				if (cur.tool != tool)
				{
					dist += 7;
				}

				if (!(nstate in distance) || dist < distance[nstate])
				{
					distance[nstate] = dist;
					nstate.priority = cast(int)dist +
						abs(nstate.x - target.x) +
						abs(nstate.y - target.y);
					queue.insert(nstate);
				}
			}
		}
	}

	return ulong.max;
}


int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s <filename>", args[0]);
		return 1;
	}

	ulong depth;
	state target;
	try
	{
		File input = File(args[1], "rb");
		input.readf!"depth: %d\n"(depth);
		input.readf!"target: %d,%d"(target.x, target.y);
		target.tool = Tools.TORCH;
		input.close();
	}
	catch (std.exception.ErrnoException)
	{
		stderr.writefln("Cannot open %s", args[1]);
		return 1;
	}
	catch (std.format.FormatException)
	{
		stderr.writeln("Cannot parse the data");
		return 1;
	}

	writefln("Part1: %d", rect_risk(depth, target));
	writefln("Part2: %d", astar(depth, target));
	return 0;
}
