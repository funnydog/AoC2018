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
	uint ipreg;
	Instruction[] program;

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
				program ~= Instruction(mnemonics[op], a, b, c);
			}
		}
		regs[] = 0;
	}

	long run(bool boost = false, long r0 = 0)
	{
		regs[] = 0;
		regs[0] = r0;
		while (0 <= regs[ipreg] && regs[ipreg] < program.length)
		{
			if (boost && regs[ipreg] == 1)
			{
				auto r = program[regs[ipreg]+3].b;
				long value = regs[r];
				long result = value + 1;
				foreach (i; 2 .. value/2+1)
				{
					if (value % i == 0)
					{
						result += i;
					}
				}
				return result;
			}
			else
			{
				auto i = &program[regs[ipreg]];
				execute(i.op, i.a, i.b, i.c);
				regs[ipreg]++;
			}
		}
		return regs[0];
	}

	void execute(int op, long a, long b, long c)
	{
		switch (op)
		{
		case ADDR: regs[c] = regs[a] + regs[b]; break;
		case ADDI: regs[c] = regs[a] + b; break;
		case MULR: regs[c] = regs[a] * regs[b]; break;
		case MULI: regs[c] = regs[a] * b; break;
		case BANR: regs[c] = regs[a] & regs[b]; break;
		case BANI: regs[c] = regs[a] & b; break;
		case BORR: regs[c] = regs[a] | regs[b]; break;
		case BORI: regs[c] = regs[a] | b; break;
		case SETR: regs[c] = regs[a]; break;
		case SETI: regs[c] = a; break;
		case GTIR: regs[c] = a > regs[b]; break;
		case GTRI: regs[c] = regs[a] > b; break;
		case GTRR: regs[c] = regs[a] > regs[b]; break;
		case EQIR: regs[c] = a == regs[b]; break;
		case EQRI: regs[c] = regs[a] == b; break;
		case EQRR: regs[c] = regs[a] == regs[b]; break;
		default: assert(0, format!"Unknown op %s"(op));
		}
	}
}

int main(string[] args)
{
	if (args.length < 2)
	{
		stderr.writefln("Usage: %s <filename", args[0]);
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

	writeln("Part1: ", s.run(true));
	writeln("Part2: ", s.run(true, 1));
	return 0;
}
