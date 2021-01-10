import std.container;
import std.string;
import std.format;
import std.stdio;
import std.typecons;

class Pair
{
	int priority;
	char value;

	this(int time, char value)
	{
		this.priority = time + 60 + value - cast(int)'A' + 1;
		this.value = value;
	}

	override int opCmp(Object rhs)
	{
		auto that = cast(Pair) rhs;
		if (!that)
		{
			return 0;
		}
		return priority - that.priority;
	}
};

class Graph
{
	int[char] degree;
	char[][char] adj;

	this(ref File input)
	{
		try
		{
			char before, after;
			while(input.readf("Step %c must be finished before step %c can begin. ",
					  &before, &after))
			{
				this.degree.require(before, 0);
				this.degree[after]++;
				this.adj.require(after, []);
				this.adj[before] ~= after;
			}
		}
		catch (FormatException e)
		{
			// exit
		}
	}

	Tuple!(string, int) schedule(ulong workers)
	{
		int time = 0;
		char[] lst;
		int[char] degree;
		BinaryHeap!(Array!char,"a>b") s;
		BinaryHeap!(Array!Pair,"a>b") jobs;
		foreach (k, v; this.degree)
		{
			degree[k] = v;
			if (v == 0)
			{
				s.insert(k);
			}
		}
		while (true)
		{
			while (!s.empty && jobs.length < workers)
			{
				auto name = s.front();
				s.removeFront();
				Pair p = new Pair(time, name);
				jobs.insert(p);
			}

			if (jobs.empty())
			{
				break;
			}

			auto p = jobs.front();
			jobs.removeFront();
			time = p.priority;
			lst ~= p.value;
			foreach(v; this.adj[p.value])
			{
				degree[v]--;
				if (degree[v] == 0)
				{
					s.insert(v);
				}
			}
		}
		return tuple(lst.idup, time);
	}
};

int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s <filename>", args[0]);
		return 1;
	}

	Graph g;
	try
	{
		File input = File(args[1], "r");
		g = new Graph(input);
		input.close();
	}
	catch (std.exception.ErrnoException)
	{
		stderr.writefln("Cannot open %s", args[1]);
		return 1;
	}

	writeln("Part1: ", g.schedule(1)[0]);
	writeln("Part2: ", g.schedule(5)[1]);
	return 0;
}
