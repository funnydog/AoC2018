#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum
{
	OP_ADDR,
	OP_ADDI,
	OP_MULR,
	OP_MULI,
	OP_BANR,
	OP_BANI,
	OP_BORR,
	OP_BORI,
	OP_SETR,
	OP_SETI,
	OP_GTIR,
	OP_GTRI,
	OP_GTRR,
	OP_EQIR,
	OP_EQRI,
	OP_EQRR,
	OP_COUNT,
};

static void compute(int *reg, int opcode, int a, int b, int c)
{
	switch (opcode)
	{
	case OP_ADDR: reg[c] = reg[a] + reg[b]; break;
	case OP_ADDI: reg[c] = reg[a] + b; break;
	case OP_MULR: reg[c] = reg[a] * reg[b]; break;
	case OP_MULI: reg[c] = reg[a] * b; break;
	case OP_BANR: reg[c] = reg[a] & reg[b];	break;
	case OP_BANI: reg[c] = reg[a] & b; break;
	case OP_BORR: reg[c] = reg[a] | reg[b]; break;
	case OP_BORI: reg[c] = reg[a] | b; break;
	case OP_SETR: reg[c] = reg[a]; break;
	case OP_SETI: reg[c] = a; break;
	case OP_GTIR: reg[c] = a > reg[b]; break;
	case OP_GTRI: reg[c] = reg[a] > b; break;
	case OP_GTRR: reg[c] = reg[a] > reg[b]; break;
	case OP_EQIR: reg[c] = a == reg[b]; break;
	case OP_EQRI: reg[c] = reg[a] == b; break;
	case OP_EQRR: reg[c] = reg[a] == reg[b]; break;
	}
}

static int process_sample(char m[OP_COUNT][OP_COUNT], int *before, int *instr, int *after)
{
	int count = 0;
	for (int i = 0; i < OP_COUNT; i++)
	{
		int reg[4];
		memmove(reg, before, sizeof(reg));
		compute(reg, i, instr[1], instr[2], instr[3]);
		if (memcmp(reg, after, sizeof(reg)) == 0)
		{
			count++;
			m[i][instr[0]] = 1;
		}
	}
	return count;
}

static int is_col_solved(const char m[OP_COUNT][OP_COUNT], int opcode)
{
	int count = 0, last = -1;
	for (int i = 0; i < OP_COUNT; i++)
	{
		if (m[i][opcode])
		{
			last = i;
		}
		count += m[i][opcode];
	}
	return count == 1 ? last : -1;
}

static void solve(char m[OP_COUNT][OP_COUNT], int opmap[OP_COUNT])
{
	memset(opmap, 0xff, sizeof(int) * OP_COUNT);
	for (int k = 0; k < OP_COUNT; k++)
	{
		int j = -1;
		for (int i = 0; i < OP_COUNT; i++)
		{
			if (opmap[i] == -1 && (j = is_col_solved(m, i)) >= 0)
			{
				opmap[i] = j;
				break;
			}
		}
		assert(j != -1);
		memset(m[j], 0, sizeof(m[j]));
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return 1;
	}

	FILE *input = fopen(argv[1], "rb");
	if (!input)
	{
		fprintf(stderr, "Cannot open %s\n", argv[1]);
		return 1;
	}

	char m[OP_COUNT][OP_COUNT];
	memset(m, 0, sizeof(m));

	size_t three_or_more = 0;
	for (;;)
	{
		int before[4];
		int instr[4];
		int after[4];
		int r = fscanf(input, " Before: [%d, %d, %d, %d]",
			       before+0, before+1, before+2, before+3);
		if (r != 4)
		{
			break;
		}
		r = fscanf(input, " %d %d %d %d",
			   instr+0, instr+1, instr+2, instr+3);
		if (r != 4)
		{
			break;
		}
		r = fscanf(input, " After: [%d, %d, %d, %d]",
			   after+0, after+1, after+2, after+3);
		if (r != 4)
		{
			break;
		}

		if (process_sample(m, before, instr, after) >= 3)
		{
			three_or_more++;
		}

	}
	printf("Part1: %zu\n", three_or_more);

	int opmap[OP_COUNT];
       	solve(m, opmap);

	/* execute the program */
	int op, a, b, c;
	int regs[4] = {0};
	while (fscanf(input, " %d %d %d %d", &op, &a, &b, &c) == 4)
	{
		compute(regs, opmap[op], a, b, c);
	}
	fclose(input);

	printf("Part2: %d\n", regs[0]);
	return 0;
}
