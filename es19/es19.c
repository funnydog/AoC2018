#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
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
	while (low < high) {
		int mid = low + (high - low)/2;
		if (strcmp(symbols[mid], symbol) < 0) {
			low = mid + 1;
		} else {
			high = mid;
		}
	}
	if (strcmp(symbols[high], symbol) == 0)
		return high;

	return -1;
}

static void execute(int64_t *reg, int opcode, int a, int b, int c)
{
	switch (opcode) {
	case OP_ADDR:
		reg[c] = reg[a] + reg[b];
		break;

	case OP_ADDI:
		reg[c] = reg[a] + b;
		break;

	case OP_MULR:
		reg[c] = reg[a] * reg[b];
		break;

	case OP_MULI:
		reg[c] = reg[a] * b;
		break;

	case OP_BANR:
		reg[c] = reg[a] & reg[b];
		break;

	case OP_BANI:
		reg[c] = reg[a] & b;
		break;

	case OP_BORR:
		reg[c] = reg[a] | reg[b];
		break;

	case OP_BORI:
		reg[c] = reg[a] | b;
		break;

	case OP_SETR:
		reg[c] = reg[a];
		break;

	case OP_SETI:
		reg[c] = a;
		break;

	case OP_GTIR:
		reg[c] = a > reg[b];
		break;

	case OP_GTRI:
		reg[c] = reg[a] > b;
		break;

	case OP_GTRR:
		reg[c] = reg[a] > reg[b];
		break;

	case OP_EQIR:
		reg[c] = a == reg[b];
		break;

	case OP_EQRI:
		reg[c] = reg[a] == b;
		break;

	case OP_EQRR:
		reg[c] = reg[a] == reg[b];
		break;

	default:
		break;
	}
}

struct cpu
{
	int64_t regs[6];
	int ip;
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

static void cpu_dump_regs(int64_t *regs)
{
	printf("[");
	for (int i = 0; i < 6; i++) {
		printf(i != 5 ? "%d, " : "%d", regs[i]);
	}
	printf("]\n");
}

static void cpu_execute(struct cpu *c, struct program *p, int boost)
{
	while (c->ip >= 0 && c->ip < p->count) {
		struct instruction *ins = p->instr + c->ip;
		c->regs[c->boundreg] = c->ip;
		if (boost && c->ip == 1) {
			/* boost replaces the instructions [1..15] */
			c->regs[4] = 1;
			while (c->regs[4] * 2 <= c->regs[2]) {
				if (c->regs[2] % c->regs[4] == 0)
					c->regs[0] += c->regs[4];
				c->regs[4]++;
			}
			c->regs[0] += c->regs[2];

			/* simulate the register values at the exit */
			c->regs[1] = 1;
			c->regs[4] = c->regs[2]+1;
			c->regs[5] = c->regs[2]+1;
			c->regs[c->boundreg] = 15;
		} else {
			execute(c->regs, ins->opcode, ins->a, ins->b, ins->c);
		}
		c->ip = c->regs[c->boundreg] + 1;
	}
	return;
}

static void program_free(struct program *p)
{
	if (p) {
		free(p->instr);
		free(p);
	}
}

static struct program *program_load(FILE *input)
{
	struct program *p = calloc(1, sizeof(*p));
	if (!p)
		return NULL;

	char *line = NULL;
	size_t linesize = 0;
	while (getline(&line, &linesize, input) != -1) {
		if (p->count == p->size) {
			size_t newsize = p->size ? p->size * 2 : 2;
			struct instruction *newi = realloc(
				p->instr, newsize * sizeof(p->instr[0]));
			if (!newi) {
				program_free(p);
				return NULL;
			}
			p->size = newsize;
			p->instr = newi;
		}

		char *space = strchr(line, ' ');
		if (!space)
			continue;

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
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return -1;
	}

	FILE *input = fopen(argv[1], "rb");
	if (!input) {
		fprintf(stderr, "Cannot open %s for reading\n", argv[1]);
		return -1;
	}

	struct cpu cpu = {0};
	fscanf(input, "#ip %d", &cpu.boundreg);
	struct program *p = program_load(input);
	/* for (size_t i = 0; i < p->count; i++) { */
	/* 	struct instruction *ins = p->instr + i; */
	/* 	printf("ip %2zu: %s %d %d %d\n", i, */
	/* 	       symbols[ins->opcode], ins->a, ins->b, ins->c); */
	/* } */
	fclose(input);

	/* cpu_execute(&cpu, p, 0); */
	/* cpu_dump_regs(cpu.regs); */
	cpu_reset(&cpu);
	cpu_execute(&cpu, p, 1);
	cpu_dump_regs(cpu.regs);
	cpu_reset(&cpu);
	cpu.regs[0] = 1;
	cpu_execute(&cpu, p, 1);
	cpu_dump_regs(cpu.regs);
	program_free(p);

	return 0;
}
