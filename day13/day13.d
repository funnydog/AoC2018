import std.algorithm.mutation;
import std.container.binaryheap;
import std.stdio;
import std.string;

struct Vec
{
	int x;
	int y;
};


class Cart
{
	enum Dir { UP, RIGHT, DOWN, LEFT };
	enum State { TURN_LEFT, FORWARD, TURN_RIGHT };

	Vec p;
	Dir d;
	State s;

	this(int x, int y, char v)
	{
		p.x = x;
		p.y = y;
		switch (v)
		{
		case '^': d = Dir.UP; break;
		case '>': d = Dir.RIGHT; break;
		case 'v': d = Dir.DOWN; break;
		case '<': d = Dir.LEFT; break;
		default: assert(0);
		}
		s = State.TURN_LEFT;
	}

	void update(in Map m)
	{
		const int[] dx = [0, 1, 0, -1];
		const int[] dy = [-1, 0, 1, 0];
		Dir[] sl = [Dir.RIGHT, Dir.UP, Dir.LEFT, Dir.DOWN];
		Dir[] bs = [Dir.LEFT, Dir.DOWN, Dir.RIGHT, Dir.UP];

		p.x += dx[d];
		p.y += dy[d];

		switch(m.data[p.y][p.x])
		{
		case '+':
			switch(s)
			{
			case State.TURN_LEFT:
				d = cast(Dir)((d + 3) % 4);
				s = State.FORWARD;
				break;
			case State.FORWARD:
				s = State.TURN_RIGHT;
				break;
			case State.TURN_RIGHT:
				d = cast(Dir)((d + 1) % 4);
				s = State.TURN_LEFT;
				break;
			default:
				assert(0);
			}
			break;

		case '\\':
			d = bs[d];
			break;

		case '/':
			d = sl[d];
			break;
		default:
			break;
		}
	}

	override int opCmp(Object other)
	{
		auto that = cast(Cart)other;
		if (!that)
		{
			return 0;
		}

		if (this.p.y != that.p.y)
		{
			return this.p.y - that.p.y;
		}
		return this.p.x - that.p.x;
	}
};

struct Map
{
	string[] data;
	Cart[] carts;

	static Map read(ref File f)
	{
		auto m = new Map;
		char[] buf;
		Cart[] carts;
		while (f.readln(buf))
		{
			// find the carts
			foreach (x, ref v; buf)
			{
				if (indexOf("^>v<", v) >= 0)
				{
					m.carts ~= new Cart(
						cast(int)x,
						cast(int)m.data.length,
						v);
					v = (v == '^' || v == 'v') ? '|' : '-';
				}
			}

			// add the line to the map
			m.data ~= buf.idup;
		}

		return *m;
	}

	void simulate(out Vec part1, out Vec part2)
	{
		// map of the positions of the carts
		bool[Vec] cartmap;
		foreach (ref c; carts)
		{
			cartmap[c.p] = true;
		}

		// binary heaps to keep the carts sorted by y and x
		// coordinates (Cart.opCmp() defines the order)
		auto source = heapify!"a>b"(carts);
		Cart[] dbuf = new Cart[carts.length];
		auto dest = BinaryHeap!(Cart[],"a>b")(dbuf, 0);

		// simulation
		bool first = true;
		ulong left = carts.length;
		while (left > 1)
		{
			while (!source.empty())
			{
				Cart c = source.front();
				source.removeFront();

				// if the cart position is not in the
				// cartmap the cart already crashed
				if (!(c.p in cartmap))
				{
					continue;
				}

				// remove the cart from the cartmap
				// and update it
				cartmap.remove(c.p);
				c.update(this);

				// check if the cart has crashed
				if (c.p in cartmap)
				{
					// save the first crash
					// position
					if (first)
					{
						first = false;
						part1 = c.p;
					}
					cartmap.remove(c.p);
					left -= 2;
				}
				else
				{
					// insert the cart position in
					// the cartmap map and the
					// cart in the queue for
					// the next tick
					cartmap[c.p] = true;
					dest.insert(c);
				}
			}
			swap(source, dest);
		}

		// find the first cart not crashed
		while (!source.empty())
		{
			Cart c = source.front();
			source.removeFront();
			if (c.p in cartmap)
			{
				part2 = c.p;
				break;
			}
		}
	}
};

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
		File input = File(args[1], "r");
		m = Map.read(input);
		input.close();
	}
	catch (std.exception.ErrnoException)
	{
		stderr.writefln("Cannot open %s", args[1]);
		return 1;
	}

	Vec part1, part2;
	m.simulate(part1, part2);
	writefln("Part1: %d,%d", part1.x, part1.y);
	writefln("Part2: %d,%d", part2.x, part2.y);
	return 0;
}
