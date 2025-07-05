#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#include "instructions.h"
#include "assembler.h"

static int is_register_name(const char *str)
{
	if (!str) return 0;
	if ((str[0] == 'R' || str[0] == 'r') &&
		isdigit((unsigned char)str[1]) &&
		str[2] == '\0')
	{
        int regnum = str[1] - '0';
        return (regnum >= 0 && regnum <= 15);
    }
    return 0;
}

/** Pass 1: scan the source, collect labels and their addresses in `table`. */
void pass1_collect_labels(const char *filename, symbol_t table[], size_t *count)
{
	FILE   *f = fopen(filename, "r");
	char    line[MAX_LINE_LENGTH];
	uint16_t pc = 0;
	*count = 0;

	if (!f) { perror("pass1: fopen"); exit(1); }

	while (fgets(line, sizeof(line), f))
	{
		// strip comment + trim
		char *c = strchr(line, ';');
		if (c) *c = '\0';
		char *p = line;
		while (isspace((unsigned char)*p)) p++;
		if (*p == '\0') continue;

		char *colon = strchr(p, ':');
		if (colon)
		{
			*colon = '\0';
			if (*count >= MAX_SYMBOLS)
			{
				fprintf(stderr, "Error: too many labels.\n");
				exit(1);
			}

			fprintf(stderr, "Warning: trailing colon in labels are non-standard:\n");
			fprintf(stderr, "as in:\n");
			fprintf(stderr, "\"%s\"\n", p);

			strncpy(table[*count].name, p, sizeof(table[*count].name)-1);
			table[*count].address = pc;
			(*count)++;
			continue;
		}

		char *first = strtok(p, " \t\r\n");
		if (!first) continue;

		int is_label = 0;

		size_t len = strlen(first);
		if (first[len-1] == ':')
		{
			first[len-1] = '\0';
			is_label = 1;
		}
		else if (!find_instruction(first) && !is_register_name(first))
		{
			is_label = 1;
		}

		if (is_label)
		{
			if (*count >= MAX_SYMBOLS)
			{
				fprintf(stderr, "Too many labels\n");
				exit(1);
			}
			strncpy(table[*count].name, first, sizeof(table[*count].name) - 1);
			table[*count].address = pc;
			(*count)++;
			continue;
		}

		const instruction *instr = find_instruction(first);
		if (!instr)
		{
			fprintf(stderr, "pass1: unknown mnemonic '%s'\n", first);
			exit(1);
		}
		pc += instr->instruction_size;
	}

	fclose(f);
}

/** Pass 2: assemble lines, resolve label operands via symbol table. */
int pass2_assemble(const char *infile,
                   const char *outfile,
                   const symbol_t table[],
                   size_t symbol_count)
{
	FILE *in = fopen(infile, "r");
	FILE *out = fopen(outfile, "wb");
	char line[MAX_LINE_LENGTH];
	int lineno = 0;

	if (!in || !out)
	{
		perror("pass2 fopen");
		if (in) fclose(in);
		if (out) fclose(out);
		return 1;
	}

	while (fgets(line, sizeof(line), in))
	{
		lineno++;
		strip_comment_and_whitespace(line);
		if (line[0] == '\0') continue;

		char line_copy[MAX_LINE_LENGTH];
		strncpy(line_copy, line, sizeof(line_copy));
		line_copy[sizeof(line_copy) - 1] = '\0';

		char *first_token = strtok(line_copy, " \t\r\n");
		if (!first_token) continue;

		size_t len = strlen(first_token);
		int is_label = 0;
		if (first_token[len - 1] == ':')
		{
			is_label = 1;
		}
		else if (!find_instruction(first_token) && !is_register_name(first_token))
		{
			is_label = 1;
		}

		if (is_label)
			continue;

		char *tokens[MAX_TOKENS] = {NULL};
		int ntok = tokenize_line(line, tokens);
		if (ntok == 0) continue;

		const char *mnemonic = tokens[0];
		char buf0[16], buf1[16];
		const char *ops[2] = {NULL, NULL};
		if (ntok > 1) ops[0] = tokens[1];
		if (ntok > 2) ops[1] = tokens[2];

		for (int i = 0; i < 2; i++)
		{
			const char *op = ops[i];
			if (!op) continue;

			if ((isalpha((unsigned char)op[0]) || op[0] == '_') && !is_register_name(op))
			{
				int found = 0;
				for (size_t j = 0; j < symbol_count; j++)
				{
					if (strcmp(op, table[j].name) == 0)
					{
						snprintf(i ? buf1 : buf0, sizeof(buf0), "%u", table[j].address);
						ops[i] = i ? buf1 : buf0;
						found = 1;
						break;
					}
				}
				if (!found)
				{
					fprintf(stderr, "Undefined label '%s' at line %d\n", op, lineno);
					fclose(in);
					fclose(out);
					return 1;
				}
			}
		}

		uint8_t bytecode[2];
		int8_t sz = assemble_instruction(mnemonic, ops, bytecode);
		if (sz < 0)
		{
			fprintf(stderr, "assemble error at line %d: %s\n", lineno, line);
			fclose(in);
			fclose(out);
			return 1;
		}

		fwrite(bytecode, 1, (size_t)sz, out);
	}

	fclose(in);
	fclose(out);
	return 0;
}

// Parses a string operand and returns its numeric value
// Supports register (R1), register pair (R0R1), and numeric formats (hex, dec, oct)
int16_t parse_operand_value(const char *str)
{
	if (!str)
		return -1;

	// Register: R1 → 1
	if ((str[0] == 'R' || str[0] == 'r') && isdigit((unsigned char)str[1]) && str[2] == '\0')
		return str[1] - '0';

	// Register pair: R0R1 → 0, R2R3 → 1, etc.
	if ((str[0] == 'R' || str[0] == 'r') &&
	    isdigit((unsigned char)str[1]) &&
	    (str[2] == 'R' || str[2] == 'r') &&
	    isdigit((unsigned char)str[3]) &&
	    str[4] == '\0')
	{
		int reg1 = str[1] - '0';
		int reg2 = str[3] - '0';
		if (reg1 % 2 == 0 && reg2 == reg1 + 1)
			return (int16_t)(reg1 / 2);
		else
			return -1;
	}

	// Convert number (supports 0x, 0, decimal)
	errno = 0;
	char *endptr;
	long value = strtol(str, &endptr, 0);
	if (errno != 0 || *endptr != '\0' || value < 0 || value > 0xFFFF)
		return -1;

	return (int16_t)value;
}

// Assembles an instruction from mnemonic and operand strings into output bytes
int8_t assemble_instruction(const char *mnemonic, const char *operands[], uint8_t output[2])
{
	const instruction *instr = find_instruction(mnemonic);
	if (!instr)
	{
		fprintf(stderr, "Error: unknown instruction: %s\n", mnemonic);
		return -1;
	}

	uint8_t opcode = instr->opcode;
	uint8_t second_byte = 0;

	for (uint8_t i = 0; i < instr->operand_count; ++i)
	{
		operand op = instr->operands[i];
		if (op == OPERAND_NONE)
			continue;

		int16_t val = parse_operand_value(operands[i]);
		if (val < 0)
		{
			fprintf(stderr, "Error: invalid operand: %s\n", operands[i]);
			return -1;
		}

		operand_info info = get_operand_info(op);

		switch (op)
		{
			case OPERAND_CONDITION:
			case OPERAND_REGISTER:
			case OPERAND_REGISTER_PAIR:
			case OPERAND_IMMEDIATE:
				uint32_t masked = ((uint32_t)val & ((1u << info.size_bits) - 1)) << info.bit_position;
				opcode |= (uint8_t)masked;
				break;

			case OPERAND_ROM_DATA:
			case OPERAND_SHORT_ADDR:
				second_byte = (uint8_t)(val & 0xFF);
				break;

			case OPERAND_LONG_ADDR:
				opcode |= (uint8_t)((val >> 8) & 0x0F);      // High 4 bits
				second_byte = (uint8_t)(val & 0xFF);         // Low 8 bits
				break;

			default:
				return -1;
		}
	}

	output[0] = opcode;
	if (instr->instruction_size == 2)
		output[1] = second_byte;

	return (int8_t)instr->instruction_size;
}

// Removes inline comments (after ';') and trims leading/trailing whitespace
void strip_comment_and_whitespace(char *line)
{
	char *comment = strchr(line, ';');
	if (comment)
		*comment = '\0';

	// Trim leading
	while (isspace((unsigned char)*line))
		line++;

	// Trim trailing
	char *end = line + strlen(line);
	while (end > line && isspace((unsigned char)*(--end)))
		*end = '\0';
}

// Splits a line into tokens (mnemonic and operands)
int tokenize_line(char *line, char *tokens[MAX_TOKENS])
{
	int count = 0;
	char *token = strtok(line, " ,\t\r\n");
	while (token && count < MAX_TOKENS)
	{
		tokens[count++] = token;
		token = strtok(NULL, " ,\t\r\n");
	}
	return count;
}

// Utility: replaces the file extension with a new one
char *replace_extension(const char *filename, const char *new_ext)
{
	char *dot = strrchr(filename, '.');
	size_t base_len = dot ? (size_t)(dot - filename) : strlen(filename);
	char *result = malloc(base_len + strlen(new_ext) + 1);
	if (!result) return NULL;

	strncpy(result, filename, base_len);
	result[base_len] = '\0';
	strcat(result, new_ext);
	return result;
}
