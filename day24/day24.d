import std.algorithm.searching;
import std.algorithm.sorting;
import std.conv;
import std.regex;
import std.stdio;
import std.string;

enum GroupType
{
	IMMUNE,
	INFECTION
};

enum DamageType
{
	NONE = 0,
	RADIATION = 1<<0,
	BLUDGEONING = 1<<1,
	FIRE = 1<<2,
	SLASHING = 1<<3,
	COLD = 1<<4,
}

class Group
{
	int type;
	int units;
	int hitpoints;
	int attack_damage;
	int attack_type;
	int initiative;

	int weakness;
	int immunity;

	Group target;
	bool selected;

	private static pattern = regex(r"\w+|\d+|[;.\(\)]");

	private DamageType get_damage(in char[] buf)
	{
		static const char[][] damage = [
			"radiation",
			"bludgeoning",
			"fire",
			"slashing",
			"cold"
		];
		foreach (i, s; damage)
		{
			if (buf == s)
			{
				return cast(DamageType)(1<<i);
			}
		}
		return DamageType.NONE;
	}

	this(int type, in char[] txt)
	{
		const (char[])[] tok;
		foreach(t; matchAll(txt, pattern))
		{
			tok ~= t.hit;
		}
		this.type = type;
		this.units = std.conv.to!(int)(tok[0]);
		this.hitpoints = std.conv.to!(int)(tok[4]);
		this.weakness = this.immunity = 0;
		ulong i = 7;
		if (tok[i] == "(")
		{
			i++;
			int *cur = null;
			for (; tok[i] != ")"; i++)
			{
				if (tok[i] == "weak")
				{
					cur = &this.weakness;
				}
				else if (tok[i] == "immune")
				{
					cur = &this.immunity;
				}
				else
				{
					*cur |= get_damage(tok[i]);
				}
			}
			i++;
		}
		this.attack_damage = std.conv.to!(int)(tok[i+5]);
		this.attack_type = get_damage(tok[i+6]);
		this.initiative = std.conv.to!(int)(tok[i+10]);
		this.target = null;
		this.selected = false;
	}

	this(in Group rhs)
	{
		this.type = rhs.type;
		this.units = rhs.units;
		this.hitpoints = rhs.hitpoints;
		this.attack_damage = rhs.attack_damage;
		this.attack_type = rhs.attack_type;
		this.initiative = rhs.initiative;
		this.weakness = rhs.weakness;
		this.immunity = rhs.immunity;
		this.target = null;
		this.selected = false;
	}

	int power() const
	{
		return units * attack_damage;
	}

	int damage(ref in Group b) const
	{
		int damage = power();
		if ((attack_type & b.immunity) != 0)
		{
			damage = 0;
		}
		else if ((attack_type & b.weakness) != 0)
		{
			damage *= 2;
		}
		return damage;
	}
}

static Group[] load(ref File input)
{
	Group[] g;
	GroupType type;
	foreach(line; input.byLine)
	{
		line = strip(line);
		if (line == "")
		{
			continue;
		}
		else if (line.startsWith("Immune"))
		{
			type = GroupType.IMMUNE;
		}
		else if (line.startsWith("Infection"))
		{
			type = GroupType.INFECTION;
		}
		else
		{
			g ~= new Group(type, line);
		}
	}
	return g;
}

static bool play(Group[] groups, int boost, out ulong units)
{
	ulong infection = 0;
	foreach(g; groups)
	{
		if (g.type == GroupType.IMMUNE)
		{
			g.attack_damage += boost;
		}
		else
		{
			infection++;
		}
	}

	bool deadlock = false;
	while (!deadlock)
	{
		// target selection
		groups.sort!(
			(in Group a, in Group b) {
				if (a.power() != b.power())
				{
					return b.power() < a.power();
				}
				return b.initiative < a.initiative;
			});
		foreach(i, a; groups)
		{
			if (a.units <= 0)
			{
				groups = groups[0..i];
				break;
			}

			int maxdamage = 0;
			a.target = null;
			foreach (b; groups)
			{
				if (b.type == a.type || b.units <= 0 || b.selected)
				{
					continue;
				}

				int damage = a.damage(b);
				if (maxdamage < damage)
				{
					maxdamage = damage;
					a.target = b;
				}
			}
			if (a.target)
			{
				a.target.selected = true;
			}
		}

		// attack
		groups.sort!((in Group a, in Group b)
			     {
				     return b.initiative < a.initiative;
			     });

		deadlock = true;
		foreach (a; groups)
		{
			a.selected = false;
			if (a.units <= 0 || a.target is null)
			{
				continue;
			}

			int dead_units = a.damage(a.target) / a.target.hitpoints;
			if (dead_units == 0)
			{
				continue;
			}

			deadlock = false;
			a.target.units -= dead_units;
			if (a.target.units <= 0 && a.type == GroupType.IMMUNE)
			{
				infection--;
			}
		}
	}

	units = 0;
	foreach (g; groups)
	{
		if (g.units > 0)
		{
			units += g.units;
		}
	}

	return infection == 0;
}

int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s <filename>", args[0]);
		return 1;
	}

	Group[] groups;
	try
	{
		File input = File(args[1], "rb");
		groups = load(input);
		input.close();
	}
	catch (std.exception.ErrnoException)
	{
		stderr.writefln("Cannot open %s", args[1]);
		return 1;
	}

	ulong units;
	auto copy = std.conv.to!(Group[])(cast (const Group[])(groups));
	play(copy, 0, units);
	writeln("Part1: ", units);

	int low = 0;
	int high = 100000;
	while (low < high)
	{
		int mid = low + (high - low) / 2;
		copy = std.conv.to!(Group[])(cast (const Group[])(groups));
		if (play(copy, mid, units))
		{
			high = mid;
		}
		else
		{
			low = mid+1;
		}
	}
	play(groups, low, units);
	writeln("Part2: ", units);
	return 0;
}
