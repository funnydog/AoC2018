#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
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

const char *symbols[] = {
	"addr",
	"addi",
	"mulr",
	"muli",
	"banr",
	"bani",
	"borr",
	"bori",
	"setr",
	"seti",
	"gtir",
	"gtri",
	"gtrr",
	"eqir",
	"eqri",
	"eqrr",
};

static void compute(int *reg, int opcode, int a, int b, int c)
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
	}
}

static int mark_opcodes(int matrix[OP_COUNT][OP_COUNT], int *before, int *instr, int *after)
{
	int count = 0;
	for (int i = 0; i < OP_COUNT; i++) {
		int reg[4];
		memmove(reg, before, sizeof(reg));
		compute(reg, i, instr[1], instr[2], instr[3]);
		if (memcmp(reg, after, sizeof(reg)) == 0) {
			count++;
		} else {
			matrix[instr[0]][i] = 0;
		}
	}
	return count;
}

static int count_opcode(int matrix[OP_COUNT][OP_COUNT], int opcode)
{
	int count = 0;
	for (int i = 0; i < OP_COUNT; i++)
		count += matrix[opcode][i];
	return count;
}

static int count_matrix(int matrix[OP_COUNT][OP_COUNT])
{
	int count = 0;
	for (int i = 0; i < OP_COUNT; i++)
		count += count_opcode(matrix, i);

	return count;
}

static void set_opmap(int opmap[OP_COUNT], int matrix[OP_COUNT][OP_COUNT])
{

	memset(opmap, 0xFF, sizeof(int) * OP_COUNT);
	while (count_matrix(matrix) > OP_COUNT) {
		for (int i = 0; i < OP_COUNT; i++) {
			if (count_opcode(matrix, i) == 1 && opmap[i] == -1) {
				for (int j = 0; j < OP_COUNT; j++) {
					if (matrix[i][j]) {
						opmap[i] = j;
						break;
					}
				}
				for (int j = 0; j < OP_COUNT; j++)
					if (i != j)
						matrix[j][opmap[i]] = 0;
			}
		}
	}
	for (int i = 0; i < OP_COUNT; i++) {
		if (opmap[i] == -1) {
			int j;
			for (j = 0; matrix[i][j] == 0; j++)
				;
			opmap[i] = j;
		}
	}
}

static void print_matrix(int mat[OP_COUNT][OP_COUNT])
{
	printf("  \\ ");
	for (int i = 0; i < OP_COUNT; i++) {
		printf(i == 0 ? "|%2d" : " %2d", i);
	}
	printf("\n----");
	for (int i = 0; i < OP_COUNT; i++) {
		printf("+--");
	}
	printf("\n");
	for (int i = 0; i < OP_COUNT; i++) {
		printf(" %2d ", i);
		for (int j = 0; j < OP_COUNT; j++) {
			printf(j == 0 ? "| %c": "  %c", mat[i][j] ? '*' : ' ');
		}
		printf("\n");
	}
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

	int mat[OP_COUNT][OP_COUNT];
	for (int i = 0; i < OP_COUNT; i++)
		for (int j = 0; j < OP_COUNT; j++)
			mat[i][j] = 1;

	int samplecount = 0;
	int samplefound = 0;
	for (;;) {
		int before[4];
		int instr[4];
		int after[4];
		int r = fscanf(input, " Before: [%d, %d, %d, %d]", before+0, before+1, before+2, before+3);
		if (r != 4)
			break;
		r = fscanf(input, " %d %d %d %d", instr+0, instr+1, instr+2, instr+3);
		if (r != 4)
			break;
		r = fscanf(input, " After: [%d, %d, %d, %d]", after+0, after+1, after+2, after+3);
		if (r != 4)
			break;

		int count = mark_opcodes(mat, before, instr, after);
		if (count >= 3)
			samplefound++;

		samplecount++;

	}
	printf("Samples that behave like 3 or more opcodes: %d/%d\n", samplefound, samplecount);

	printf("\nMatrix before simplification:\n");
	print_matrix(mat);
	int opmap[OP_COUNT];
       	set_opmap(opmap, mat);
	printf("\nMatrix after simplification:\n");
	print_matrix(mat);
	printf("\nOpcode map:\n");
	for (int i = 0; i < OP_COUNT; i++)
		printf("%s = %2d\n", symbols[i], opmap[i]);

	/* execute the program */
	printf("\nProgram execution:\n");
	int op, a, b, c;
	int regs[4] = {0};
	while (fscanf(input, " %d %d %d %d", &op, &a, &b, &c) == 4) {
		printf("%s %d %d %d", symbols[opmap[op]], a, b, c);
		compute(regs, opmap[op], a, b, c);
		printf(" %4d %4d %4d %4d\n", regs[0], regs[1], regs[2], regs[3]);
	}
	printf("Value in register 0: %d\n", regs[0]);

	fclose(input);
	return 0;
}
