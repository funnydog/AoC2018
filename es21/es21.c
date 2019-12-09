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

	STACKSIZE = 1000,
	ELSIZE = 100000,
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

static void execute(int *reg, int opcode, int a, int b, int c)
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
	int regs[6];
	int ip;
	int boundreg;

	int stack[STACKSIZE];
	size_t scount;

	int el[ELSIZE];
	size_t ecount;
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

static void cpu_reset(struct cpu *c, int r0)
{
	int boundreg = c->boundreg;
	memset(c, 0, sizeof(*c));
	c->boundreg = boundreg;
	c->regs[0] = r0;
	c->scount = c->ecount = 0;
}

static void cpu_dump_regs(int *regs)
{
	printf("[");
	for (int i = 0; i < 6; i++) {
		printf(i != 5 ? "%d, " : "%d", regs[i]);
	}
	printf("]\n");
}

static int cpu_check_cycle(struct cpu *c, int value)
{
	while (c->scount && c->stack[c->scount-1] < value)
		c->scount--;

	if (c->scount && c->stack[c->scount-1] == value)
		return 1;

	if (c->scount == STACKSIZE)
		abort();

	c->stack[c->scount++] = value;
	return 0;
}

static int cpu_cycle_last(struct cpu *c)
{
	size_t i, j = c->ecount-1;
	for (i = 0; i < c->ecount; i++)
		if (c->el[i] == c->el[j])
			break;

	if (i == c->ecount)
		return -1;

	while (i > 0 && c->el[i] == c->el[j]) {
		i--;
		j--;
	}
	return c->el[j];
}

static void cpu_execute(struct cpu *c, struct program *p)
{
	while (c->ip >= 0 && c->ip < p->count) {
		struct instruction *ins = p->instr + c->ip;
		if (c->ip == 7) {
			if (c->ecount == ELSIZE)
				abort();
			c->el[c->ecount++] = c->regs[2];
			if (cpu_check_cycle(c, c->regs[2]))
				return;
		}
		c->regs[c->boundreg] = c->ip;
		if (c->ip == 17) {
			/* boost these instructions */
			c->regs[4] = c->regs[1] = c->regs[1] / 256;
			c->regs[3] = 1;
			c->regs[c->boundreg] = 7;
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
	fclose(input);

	cpu_reset(&cpu, 0);
	cpu_execute(&cpu, p);

	printf("Answer1: %d\n", cpu.el[1]);
	printf("Answer2: %d\n", cpu_cycle_last(&cpu));

	program_free(p);
	return 0;
}
