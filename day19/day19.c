#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum
{
	OP_ADDI,
	OP_ADDR,
	OP_BANI,
	OP_BANR,
	OP_BORI,
	OP_BORR,
	OP_EQIR,
	OP_EQRI,
	OP_EQRR,
	OP_GTIR,
	OP_GTRI,
	OP_GTRR,
	OP_MULI,
	OP_MULR,
	OP_SETI,
	OP_SETR,
	OP_COUNT,
};

static const char *symbols[] = {
	"addi",
	"addr",
	"bani",
	"banr",
	"bori",
	"borr",
	"eqir",
	"eqri",
	"eqrr",
	"gtir",
	"gtri",
	"gtrr",
	"muli",
	"mulr",
	"seti",
	"setr",
};

static int find_opcode(const char *symbol)
{
	int low = 0;
	int high = OP_COUNT-1;
	while (low < high)
	{
		int mid = low + (high - low)/2;
		if (strcmp(symbols[mid], symbol) < 0)
		{
			low = mid + 1;
		}
		else
		{
			high = mid;
		}
	}

	if (strcmp(symbols[high], symbol) == 0)
	{
		return high;
	}

	return -1;
}

static void execute(int64_t *reg, int opcode, int a, int b, int c)
{
	switch (opcode)
	{
	case OP_ADDR: reg[c] = reg[a] + reg[b]; break;
	case OP_ADDI: reg[c] = reg[a] + b; break;
	case OP_MULR: reg[c] = reg[a] * reg[b]; break;
	case OP_MULI: reg[c] = reg[a] * b; break;
	case OP_BANR: reg[c] = reg[a] & reg[b]; break;
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
	default: assert(0); break;
	}
}

struct cpu
{
	int64_t regs[6];
	int boundreg;
};

struct program
{
	struct instruction
	{
		int opcode;
		int a;
		int b;
		int c;
	} *instr;
	size_t count;
	size_t size;
};

static void cpu_reset(struct cpu *c)
{
	int boundreg = c->boundreg;
	memset(c, 0, sizeof(*c));
	c->boundreg = boundreg;
}

static int64_t cpu_execute(struct cpu *c, struct program *p, int boost)
{
	size_t ip = 0;
	while (ip < p->count)
	{
		struct instruction *ins = p->instr + ip;
		if (boost && ip == 1)
		{
			int r = p->instr[ip + 3].b;
			int64_t value = c->regs[r];
			int64_t result = value + 1;
			for (int64_t i = 2, e = value / 2 + 1; i < e; i++)
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
			execute(c->regs, ins->opcode, ins->a, ins->b, ins->c);
		}
		ip = ++c->regs[c->boundreg];
	}
	return c->regs[0];
}

static void program_free(struct program *p)
{
	if (p)
	{
		free(p->instr);
		free(p);
	}
}

static struct program *program_load(FILE *input)
{
	struct program *p = calloc(1, sizeof(*p));
	if (!p)
	{
		return NULL;
	}

	char *line = NULL;
	size_t linesize = 0;
	while (getline(&line, &linesize, input) != -1)
	{
		if (p->count == p->size)
		{
			size_t newsize = p->size ? p->size * 2 : 2;
			struct instruction *newi = realloc(
				p->instr, newsize * sizeof(p->instr[0]));
			if (!newi)
			{
				program_free(p);
				return NULL;
			}
			p->size = newsize;
			p->instr = newi;
		}

		char *space = strchr(line, ' ');
		if (!space)
		{
			continue;
		}

		*space = 0;
		p->instr[p->count].opcode = find_opcode(line);
		sscanf(space+1, "%d %d %d",
		       &p->instr[p->count].a,
		       &p->instr[p->count].b,
		       &p->instr[p->count].c);
		p->count++;
	}
	free(line);

	return p;
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

	struct cpu cpu = {0};
	fscanf(input, "#ip %d", &cpu.boundreg);
	struct program *p = program_load(input);
	fclose(input);
	if (!p)
	{
		fprintf(stderr, "Cannot parse the data\n");
		return 1;
	}

	printf("Part1: %" PRId64 "\n", cpu_execute(&cpu, p, 1));

	cpu_reset(&cpu);
	cpu.regs[0] = 1;
	printf("Part2: %" PRId64 "\n", cpu_execute(&cpu, p, 1));
	program_free(p);

	return 0;
}
