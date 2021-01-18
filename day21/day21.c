#include <assert.h>
#include <limits.h>
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

const char *symbols[] = {
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

struct cpu
{
	int regs[6];
	int ipreg;
	int divreg;
	struct program *p;
	size_t cycles;
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

static void cpu_execute(struct cpu *cp)
{
	while (0 <= cp->regs[cp->ipreg] && cp->regs[cp->ipreg] < cp->p->count)
	{
		if (cp->regs[cp->ipreg] == 17)
		{
			/* NOTE: the subroutine just divides the
			 * register by 256, so do it directly and
			 * return */
			cp->regs[cp->divreg] /= 256;
			cp->regs[cp->ipreg] = 8;
		}
		int *reg = cp->regs;
		struct instruction *i = cp->p->instr + cp->regs[cp->ipreg];
		int a = i->a;
		int b = i->b;
		int c = i->c;
		switch (i->opcode)
		{
		case OP_ADDR: reg[c] = reg[a] + reg[b]; break;
		case OP_ADDI: reg[c] = reg[a] + b; break;
		case OP_MULR: reg[c] = reg[a] * reg[b];	break;
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

		cp->regs[cp->ipreg]++;
		if (cp->regs[cp->ipreg] == 30)
		{
			break;
		}
	}
}

static void cpu_find_values(struct cpu *c, struct program *p, int *part1, int *part2)
{
	/* reset the cpu */
	int ipreg = c->ipreg;
	memset(c, 0, sizeof(*c));
	c->ipreg = ipreg;
	c->p = p;
	assert(30 < p->count);

	/* find the generator and dividend registers */
	c->divreg = p->instr[26].c;
	int r = p->instr[7].c;

	/* find the cycle with Floyd's algorithm */
	struct cpu tortoise = *c;
	struct cpu hare = *c;
	cpu_execute(&tortoise);
	cpu_execute(&hare);
	cpu_execute(&hare);
	*part1 = tortoise.regs[r];
	while (tortoise.regs[r] != hare.regs[r])
	{
		cpu_execute(&tortoise);
		cpu_execute(&hare);
		cpu_execute(&hare);
	}

	tortoise = *c;
	while (tortoise.regs[r] != hare.regs[r])
	{
		cpu_execute(&tortoise);
		cpu_execute(&hare);
	}

	hare = tortoise;
	do
	{
		*part2 = hare.regs[r];
		cpu_execute(&hare);
	} while (tortoise.regs[r] != hare.regs[r]);
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
	fscanf(input, "#ip %d", &cpu.ipreg);
	struct program *p = program_load(input);
	fclose(input);

	int part1, part2;
	cpu_find_values(&cpu, p, &part1, &part2);
	program_free(p);

	printf("Part1: %d\nPart2: %d\n", part1, part2);
	return 0;
}
