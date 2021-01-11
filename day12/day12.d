import std.stdio;
import std.string;
import std.algorithm.searching;

class State
{
	int pos;
	string data;

	this()
	{
	}

	this(string data)
	{
		this.data = data.dup;
	}

	State step(in bool[string] rules)
	{
		string tmp = "...." ~ data ~ "....";
		char[] n;
		for (ulong i = 0; i < tmp.length - 4; i++)
		{
			if (tmp[i .. i+5] in rules)
			{
				n ~= "#";
			}
			else
			{
				n ~= ".";
			}
		}
		pos += n.indexOf("#") - 2;
		data = strip(n, ".").idup;
		return this;
	}

	long count() const
	{
		long count = 0;
		foreach(i, v; data)
		{
			if (v == '#')
			{
				count += pos+i;
			}
		}
		return count;
	}

	State dup() const
	{
		return new State(data);
	}
};

void parse(File input, out State s, out bool[string] rules)
{
	s = new State;
	char[] buf;
	while (input.readln(buf))
	{
		buf = buf.strip();
		if (buf.empty())
		{
			// nothing
		}
		else if (buf.startsWith("initial state: "))
		{
			s.data = buf[15..$].idup;
			s.pos = 0;
		}
		else if (buf[$-1] == '.')
		{
			// ignore
		}
		else
		{
			rules[buf[0..5].idup] = true;
		}
	}
}

long part1(in State state, in bool[string] rules)
{
	State n = state.dup;
	foreach (i; 0..20)
	{
		n.step(rules);
	}
	return n.count();
}

long part2(in State state, in bool[string] rules)
{
	State tortoise = state.dup;
	State hare = state.dup;
	long i = 0;
	do
	{
		tortoise.step(rules);
		hare.step(rules).step(rules);
		i++;
	} while (tortoise.data != hare.data);

	long cur = tortoise.count();
	long next = tortoise.step(rules).count();
	return cur + (50000000000 - i) * (next - cur);
}

int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s <filename>", args[0]);
		return 1;
	}

	State s;
	bool[string] rules;
	try
	{
		File input = File(args[1], "r");
		parse(input, s, rules);
		input.close();
	}
	catch (std.exception.ErrnoException)
	{
		stderr.writefln("Cannot open %s", args[1]);
		return 1;
	}

	writeln("Part1: ", part1(s, rules));
	writeln("Part2: ", part2(s, rules));
	return 0;
}
