import std.format;
import std.stdio;

enum
{
	ADDR,
	ADDI,
	MULR,
	MULI,
	BANR,
	BANI,
	BORR,
	BORI,
	SETR,
	SETI,
	GTIR,
	GTRI,
	GTRR,
	EQIR,
	EQRI,
	EQRR,
	OPCOUNT,
};

static void execute(int op, int a, int b, int c, int[] reg)
{
	switch (op)
	{
	case ADDR: reg[c] = reg[a] + reg[b]; break;
	case ADDI: reg[c] = reg[a] + b; break;
	case MULR: reg[c] = reg[a] * reg[b]; break;
	case MULI: reg[c] = reg[a] * b; break;
	case BANR: reg[c] = reg[a] & reg[b]; break;
	case BANI: reg[c] = reg[a] & b; break;
	case BORR: reg[c] = reg[a] | reg[b]; break;
	case BORI: reg[c] = reg[a] | b; break;
	case SETR: reg[c] = reg[a]; break;
	case SETI: reg[c] = a; break;
	case GTIR: reg[c] = a > reg[b]; break;
	case GTRI: reg[c] = reg[a] > b; break;
	case GTRR: reg[c] = reg[a] > reg[b]; break;
	case EQIR: reg[c] = a == reg[b]; break;
	case EQRI: reg[c] = reg[a] == b; break;
	case EQRR: reg[c] = reg[a] == reg[b]; break;
	default: assert(0, format!"Unknown op %d"(op));
	}
}

static ulong process_sample(int[] b, int[] i, int[] a, ref char[OPCOUNT][OPCOUNT]m)
{
	ulong count = 0;
	for (int op = 0; op < OPCOUNT; op++)
	{
		int[4] reg = b[0..4];
		execute(op, i[1], i[2], i[3], reg);
		if (reg == a)
		{
			m[op][i[0]] = 1;
			count++;
		}
	}
	return count;
}

static ulong read_samples(ref File input, out char[OPCOUNT][OPCOUNT] m)
{
	ulong three_or_more = 0;
	foreach (ref r; m)
	{
		r[] = 0;
	}
	while (true)
	{
		try
		{
			int[4] b;
			input.readf!"Before: [%d, %d, %d, %d]\n"(b[0], b[1], b[2], b[3]);

			int[4] i;
			input.readf!"%d %d %d %d\n"(i[0], i[1], i[2], i[3]);

			int[4] a;
			input.readf!"After:  [%d, %d, %d, %d]\n"(a[0], a[1], a[2], a[3]);

			input.readln();
			if (process_sample(b, i, a, m) >= 3)
			{
				three_or_more++;
			}
		}
		catch (std.conv.ConvException)
		{
			break;
		}
	}
	return three_or_more;
}

static int is_col_solved(ref in char[OPCOUNT][OPCOUNT] m, ulong x)
{
	ulong count = 0;
	int last;
	foreach(y; 0 .. OPCOUNT)
	{
		if (m[y][x])
		{
			count++;
			last = y;
		}
	}
	return count == 1 ? last : -1;
}

static void solve(ref char[OPCOUNT][OPCOUNT] m, out int[OPCOUNT] opmap)
{
	opmap[] = -1;
	foreach(_; 0 .. OPCOUNT)
	{
		int j = -1;
		foreach(x; 0 .. OPCOUNT)
		{
			if (opmap[x] == -1 && (j = is_col_solved(m, x))>=0)
			{
				opmap[x] = j;
				break;
			}
		}
		assert(j != -1, "Cannot eliminate the constraints");
		m[j] = 0;
	}
}

static int run_program(ref File input, in int[] opmap)
{
	int[4] regs = 0;
	try
	{
		int op, a, b, c;
		input.readln();
		input.readln();
		input.readln();
		while (input.readf!"%d %d %d %d "(op, a, b, c))
		{
			execute(opmap[op], a, b, c, regs);
		}
	}
	catch (std.conv.ConvException)
	{
	}
	return regs[0];
}

int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s <filename>", args[0]);
		return 1;
	}

	ulong part1;
	int part2;
	try
	{
		File input = File(args[1], "r");
		char[OPCOUNT][OPCOUNT] m;
		part1 = read_samples(input, m);
		int[OPCOUNT] opmap;
		solve(m, opmap);
		part2 = run_program(input, opmap);
		input.close();
	}
	catch (std.exception.ErrnoException e)
	{
		stderr.writefln("Cannot open %s", args[1]);
		return 1;
	}

	writeln("Part1: ", part1);
	writeln("Part2: ", part2);
	return 0;
}
