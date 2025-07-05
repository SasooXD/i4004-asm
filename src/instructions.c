#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#include "instructions.h"

// Returns the size and position of the operand in the instruction based on the type
operand_info get_operand_info(operand type)
{
	// See comments in the operand enum in the header for more info
	switch (type)
	{
		case OPERAND_CONDITION: return (operand_info){ .bit_position = 0, .size_bits = 4 };
		case OPERAND_REGISTER: return (operand_info){ .bit_position = 0, .size_bits = 4 };
		case OPERAND_REGISTER_PAIR: return (operand_info){ .bit_position = 1, .size_bits = 3 };
		case OPERAND_IMMEDIATE: return (operand_info){ .bit_position = 0, .size_bits = 4 };
		case OPERAND_ROM_DATA: return (operand_info){ .bit_position = 8, .size_bits = 8 };
		case OPERAND_SHORT_ADDR: return (operand_info){ .bit_position = 8, .size_bits = 8 };
		case OPERAND_LONG_ADDR: return (operand_info){ .bit_position = 8, .size_bits = 12 };
		case OPERAND_NONE:
		default: return (operand_info){ .bit_position = 0,.size_bits = 0 };
	}
}

// Returns a pointer to the correct entry in the instruction table based on a mnemonic
const instruction* find_instruction(const char *mnemonic)
{
	for (size_t i = 0; i < sizeof(instructions) / sizeof(instruction); ++i)
	{
		if (strcmp(instructions[i].mnemonic, mnemonic) == 0)
			return &instructions[i];
	}

	return NULL;
}
