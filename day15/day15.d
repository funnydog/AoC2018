import std.algorithm;
import std.container;
import std.format;
import std.stdio;
import std.conv;

struct Pos
{
	int x;
	int y;

	Pos opBinary(string op)(in Pos rhs) const
	{
		static if (op == "+") return Pos(x+rhs.x, y+rhs.y);
		else static assert(0, "Operator " ~ op ~ " not implemented");
	}
};

class Unit
{
	char cls;
	Pos pos;
	int attack;
	int hp;

	this(char cls, int x, int y)
	{
		this.cls = cls;
		this.pos.x = x;
		this.pos.y = y;
		this.attack = 3;
		this.hp = 200;
	}

	this(const Unit u)
	{
		this.cls = u.cls;
		this.pos = u.pos;
		this.attack = u.attack;
		this.hp = u.hp;
	}

	override int opCmp(Object other)
	{
		auto that = cast(Unit)other;
		assert(that);
		if (this.pos.y != that.pos.y)
		{
			return this.pos.y - that.pos.y;
		}
		return this.pos.x - that.pos.x;
	}
};

class Map
{
	char[][] data;
	ulong[][] dist;
	Unit[] units;
	ulong elves;
	ulong goblins;

	static const Pos[] off = [Pos(0, -1), Pos(-1, 0), Pos(1, 0), Pos(0, 1)];

	static Map read(ref File input)
	{
		auto m = new Map;
		char[] buf;
		while (input.readln(buf))
		{
			buf = buf.strip('\n');
			foreach (x, c; buf)
			{
				switch (c)
				{
				case 'E':
					m.elves++;
					break;
				case 'G':
					m.goblins++;
					break;
				default:
					continue;
				}
				m.units ~= new Unit(
					c,
					cast(int)x,
					cast(int)m.data.length);
			}
			m.data ~= buf.dup;
		}
		m.dist = new ulong[][](m.data.length, m.data[0].length);
		return m;
	}

	override string toString() const
	{
		string buf;
		ulong i = 0;
		foreach(y, row; data)
		{
			buf ~= format!"%s"(row);
			for (; i < units.length && units[i].pos.y == y; i++)
			{
				const Unit u = units[i];
				if (u.hp > 0 && u.pos.y == y)
				{
					buf ~= format!" %c(%d)"(u.cls, u.hp);
				}
			}
			buf ~= "\n";
		}
		return buf;
	}

	Map dup() const
	{
		auto m = new Map;
		m.data = data.to!(char[][]);
		m.units = units.to!(Unit[]);
		m.elves = elves;
		m.goblins = goblins;
		m.dist = dist.to!(ulong[][]);
		return m;
	}

	void bfs(Pos p)
	{
		for (ulong y = 0; y < data.length; y++)
		{
			for (ulong x = 0; x < data[y].length; x++)
			{
				dist[y][x] = ulong.max;
			}
		}

		auto queue = DList!Pos();
		queue.insertBack(p);
		dist[p.y][p.x] = 0;
		while (!queue.empty())
		{
			p = queue.front();
			queue.removeFront();
			foreach (o; off)
			{
				Pos n = p + o;
				if (data[n.y][n.x] != '.')
				{
					continue;
				}
				if (dist[n.y][n.x] == ulong.max)
				{
					dist[n.y][n.x] = dist[p.y][p.x]+1;
					queue.insertBack(n);
				}
			}
		}
	}

	void move(Unit u)
	{
		// check if already in range of an enemy
		auto enemy = u.cls == 'E' ? 'G' : 'E';
		foreach(o; off)
		{
			Pos p = u.pos + o;
			if (data[p.y][p.x] == enemy)
			{
				return;
			}
		}

		// find the destination for the unit
		bfs(u.pos);
		ulong mind = ulong.max;
		Pos dest;
		foreach (e; units)
		{
			if (e.cls != enemy || e.hp <= 0)
			{
				continue;
			}
			foreach(o; off)
			{
				Pos p = e.pos + o;
				if (mind > dist[p.y][p.x])
				{
					mind = dist[p.y][p.x];
					dest = p;
				}
			}
		}

		// no destination available
		if (mind == ulong.max)
		{
			return;
		}

		// walk backwards towards the unit
		while (true)
		{
			Pos minp = dest;
			foreach(o; off)
			{
				Pos p = dest + o;
				if (mind > dist[p.y][p.x])
				{
					mind = dist[p.y][p.x];
					minp = p;
				}
			}
			if (minp == u.pos)
			{
				break;
			}
			dest = minp;
		}

		// update the map and the unit
		data[u.pos.y][u.pos.x] = '.';
		data[dest.y][dest.x] = u.cls;
		u.pos = dest;
	}

	bool attack(Unit u)
	{
		int minhp = int.max;
		Unit target = null;
		Pos[] adj;
		foreach(o; off)
		{
			adj ~= u.pos + o;
		}
		foreach(e; units)
		{
			// don't fight friends or dead units
			if (e.cls == u.cls || e.hp <= 0)
			{
				continue;
			}

			foreach(n; adj)
			{
				if (n == e.pos && minhp > e.hp)
				{
					minhp = e.hp;
					target = e;
				}
			}
		}

		if (minhp != int.max)
		{
			target.hp -= u.attack;
			if (target.hp <= 0)
			{
				data[target.pos.y][target.pos.x] = '.';
				return true;
			}
		}
		return false;
	}

	ulong play(bool return_early = false)
	{
		for (ulong i = 0; ; i++)
		{
			units.sort();
			foreach(u; units)
			{
				//skip dead units
				if (u.hp <= 0)
				{
					continue;
				}

				move(u);
				if (attack(u))
				{
					if (u.cls == 'G')
					{
						if (return_early || --elves == 0)
						{
							return i;
						}
					}
					else if (--goblins == 0)
					{
						return i;
					}
				}
			}
		}
	}

	ulong points() const
	{
		ulong points = 0;
		foreach(u; units)
		{
			if (u.hp > 0)
			{
				points += u.hp;
			}
		}
		return points;
	}
};

int main(string[] args)
{
	if (args.length<2)
	{
		stderr.writefln("Usage: %s <filename>", args[0]);
		return 1;
	}

	Map m;
	try
	{
		File input = File(args[1], "r");
		m = Map.read(input);
		input.close;
	}
	catch (std.exception.ErrnoException e)
	{
		stderr.writefln("Cannot open %s", args[1]);
		return 1;
	}

	auto m1 = m.dup;
	writeln("Part1: ", m1.play() * m1.points());

	// bisect the minimum level of attack to let the elves win
	int low = 4;
	int high = 200;
	while (low < high)
	{
		int mid = low + (high - low) / 2;
		m1 = m.dup;
		foreach(u; m1.units)
		{
			if (u.cls == 'E') u.attack = mid;
		}
		m1.play(true);
		if (m1.goblins == 0)
		{
			high = mid;
		}
		else
		{
			low = mid + 1;
		}
	}

	// play one last game with the attack level found
	foreach(u; m.units)
	{
		if (u.cls == 'E') u.attack = high;
	}
	writeln("Part2: ", m.play() * m.points());
	return 0;
}
