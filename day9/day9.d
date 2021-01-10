import std.algorithm.searching;
import std.format;
import std.stdio;

ulong game(ulong marbles, ulong players)
{
	marbles++;
	ulong[] next = new ulong[marbles];
	ulong[] prev = new ulong[marbles];
	ulong[] score = new ulong[players];

	next[0] = prev[0] = 0;
	ulong cur = 0;
	ulong player = 0;
	for (ulong i = 1; i < marbles; i++)
	{
		if (i % 23 == 0)
		{
			ulong pos = cur;
			foreach (_; 0..7)
			{
				pos = prev[pos];
			}
			next[prev[pos]] = next[pos];
			prev[next[pos]] = prev[pos];
			player %= players;
			score[player] += pos + i;
			cur = next[pos];
		}
		else
		{
			ulong pos1 = next[cur];
			ulong pos2 = next[pos1];
			next[i] = pos2;
			prev[i] = pos1;
			next[pos1] = i;
			prev[pos2] = i;
			cur = i;
		}
		player++;
	}
	return score.maxElement;
}

int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s <filename>", args[0]);
		return 11;
	}

	ulong numplayers, lastmarble;
	try
	{
		File input = File(args[1], "r");
		input.readf("%d players; last marble is worth %d points",
			    &numplayers, &lastmarble);
		input.close();
	}
	catch (std.exception.ErrnoException e)
	{
		stderr.writefln("Cannot open %s for reading", args[1]);
		return 1;
	}
	catch (FormatException e)
	{
		stderr.writefln("Cannot parse the data");
		return 1;
	}

	writeln("Part1: ", game(lastmarble, numplayers));
	writeln("Part2: ", game(lastmarble*100, numplayers));
	return 0;
}
