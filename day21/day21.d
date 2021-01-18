import std.conv;
import std.stdio;
import std.format;

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

struct Instruction
{
	int op;
	long a;
	long b;
	long c;
}

class System
{
	long[6] regs;
	long ipreg;
	long divreg;
	long genreg;
	const Instruction[] program;

	this (ref File input)
	{
		int[string] mnemonics = [
			"addr": ADDR,
			"addi": ADDI,
			"mulr": MULR,
			"muli": MULI,
			"banr": BANR,
			"bani": BANI,
			"borr": BORR,
			"bori": BORI,
			"setr": SETR,
			"seti": SETI,
			"gtir": GTIR,
			"gtri": GTRI,
			"gtrr": GTRR,
			"eqir": EQIR,
			"eqri": EQRI,
			"eqrr": EQRR,
			];
		Instruction[] p;
		foreach (line; input.byLine)
		{
			if (line[0] == '#')
			{
				line.formattedRead!"#ip %s"(ipreg);
			}
			else
			{
				string op;
				long a, b, c;
				line.formattedRead!"%s %d %d %d"(op, a, b, c);
				p ~= Instruction(mnemonics[op], a, b, c);
			}
		}
		assert(p.length>30, "Unsuitable program");
		program = p;
		divreg = p[26].c;
		genreg = p[7].c;
		regs[] = 0;
	}

	this(in System other)
	{
		regs = std.conv.to!(long[])(other.regs);
		ipreg = other.ipreg;
		divreg = other.divreg;
		genreg = other.genreg;
		program = other.program;
	}

	System dup() const
	{
		return new System(this);
	}

	long execute()
	{
		while (0 <= regs[ipreg] && regs[ipreg] < program.length)
		{
			if (regs[ipreg] == 17)
			{
				regs[divreg] /= 256;
				regs[ipreg] = 8;
			}
			auto i = &program[regs[ipreg]];
			switch (i.op)
			{
			case ADDR: regs[i.c] = regs[i.a] + regs[i.b]; break;
			case ADDI: regs[i.c] = regs[i.a] + i.b; break;
			case MULR: regs[i.c] = regs[i.a] * regs[i.b]; break;
			case MULI: regs[i.c] = regs[i.a] * i.b; break;
			case BANR: regs[i.c] = regs[i.a] & regs[i.b]; break;
			case BANI: regs[i.c] = regs[i.a] & i.b; break;
			case BORR: regs[i.c] = regs[i.a] | regs[i.b]; break;
			case BORI: regs[i.c] = regs[i.a] | i.b; break;
			case SETR: regs[i.c] = regs[i.a]; break;
			case SETI: regs[i.c] = i.a; break;
			case GTIR: regs[i.c] = i.a > regs[i.b]; break;
			case GTRI: regs[i.c] = regs[i.a] > i.b; break;
			case GTRR: regs[i.c] = regs[i.a] > regs[i.b]; break;
			case EQIR: regs[i.c] = i.a == regs[i.b]; break;
			case EQRI: regs[i.c] = regs[i.a] == i.b; break;
			case EQRR: regs[i.c] = regs[i.a] == regs[i.b]; break;
			default: assert(0, format!"Unknown op %s"(i.op));
			}
			regs[ipreg]++;
			if (regs[ipreg] == 30)
			{
				break;
			}
		}
		return regs[genreg];
	}

	override bool opEquals(Object o) const
	{
		auto that = cast(System)o;
		if (!o)
		{
			return false;
		}
		return this.regs[this.genreg] == that.regs[that.genreg];
	}
}

static void solve(in System s, out long part1, out long part2)
{
	auto tortoise = s.dup;
	auto hare = s.dup;

	// NOTE: use Floyd's algorithm to find the last element of the
	// cycle.
	part1 = tortoise.execute();
	hare.execute();
	hare.execute();
	while (tortoise != hare)
	{
		tortoise.execute();
		hare.execute();
		hare.execute();
	}

	tortoise = s.dup;
	while (tortoise != hare)
	{
		tortoise.execute();
		hare.execute();
	}

	hare = tortoise.dup;
	do
	{
		part2 = hare.regs[hare.genreg];
		hare.execute();
	} while (tortoise != hare);
}

int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s <filename>", args[0]);
		return 1;
	}

	System s;
	try
	{
		File input = File(args[1], "rb");
		s = new System(input);
		input.close();
	}
	catch (std.exception.ErrnoException)
	{
		stderr.writefln("Cannot open %s", args[1]);
		return 1;
	}

	long part1, part2;
	solve(s, part1, part2);

	writeln("Part1: ", part1);
	writeln("Part2: ", part2);
	return 0;
}
