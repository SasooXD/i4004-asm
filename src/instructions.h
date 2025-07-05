// Operand types
typedef enum : uint8_t
{
	OPERAND_NONE = 0, // Fake operand
	OPERAND_CONDITION, // c: condition (4 bits)
	OPERAND_REGISTER, // r: register (4 bits)
	OPERAND_REGISTER_PAIR, // R: register pair (3 bits, bit pos 1)
	OPERAND_IMMEDIATE, // d: immediate data (4 bits)
	OPERAND_ROM_DATA, // D: ROM data (8 bits, pos 8)
	OPERAND_SHORT_ADDR, // a: short ROM address (8 bits, pos 8)
	OPERAND_LONG_ADDR // A: long ROM address (12 bits, pos 8)
}
operand;

// Operand info
typedef struct
{
	uint8_t bit_position;
	uint8_t size_bits;
}
operand_info;

// Instruction data
typedef struct
{
	const char *mnemonic;
	uint8_t opcode;
	uint8_t instruction_size;
	operand operands[2];
	uint8_t operand_count;
}
instruction;

// Macro for no operand instruction
#define INSTRUCTION_0_OP(mnemonic, opcode) \
	{ mnemonic, opcode, 1, {OPERAND_NONE, OPERAND_NONE}, 0 }

// Macro for single operand instruction
#define INSTRUCTION_1_OP(mnemonic, opcode, operand1) \
	{ mnemonic, opcode, 1, {operand1, OPERAND_NONE}, 1 }

// Macro for two operand instruction
#define INSTRUCTION_2_OP(mnemonic, opcode, operand1, operand2) \
	{ mnemonic, opcode, 2, {operand1, operand2}, 2 }

// Instruction table
static const instruction instructions[] =
{
	INSTRUCTION_0_OP("NOP", 0x00), // No operation
	INSTRUCTION_2_OP("JCN", 0x10, OPERAND_CONDITION, OPERAND_SHORT_ADDR), // Conditional jump
	INSTRUCTION_2_OP("FIM", 0x20, OPERAND_REGISTER_PAIR, OPERAND_ROM_DATA), // Load register pair
	INSTRUCTION_1_OP("SRC", 0x21, OPERAND_REGISTER_PAIR), //
	INSTRUCTION_1_OP("FIN", 0x30, OPERAND_REGISTER_PAIR), // Load register pair from ROM
	INSTRUCTION_1_OP("JIN", 0x31, OPERAND_REGISTER_PAIR), // Jump using register pair
	INSTRUCTION_2_OP("JUN", 0x40, OPERAND_LONG_ADDR, OPERAND_NONE), // Unconditional jump
	INSTRUCTION_2_OP("JMS", 0x50, OPERAND_LONG_ADDR, OPERAND_NONE), // Jump to subroutine
	INSTRUCTION_1_OP("INC", 0x60, OPERAND_REGISTER), // Increment register
	INSTRUCTION_2_OP("ISZ", 0x70, OPERAND_REGISTER, OPERAND_SHORT_ADDR), //
	INSTRUCTION_1_OP("ADD", 0x80, OPERAND_REGISTER), // Addition with register
	INSTRUCTION_1_OP("SUB", 0x90, OPERAND_REGISTER), // Subtraction with register
	INSTRUCTION_1_OP("LD",  0xA0, OPERAND_REGISTER), // Load register into accumulator
	INSTRUCTION_1_OP("XCH", 0xB0, OPERAND_REGISTER), // Exchange accumulator with register
	INSTRUCTION_1_OP("BBL", 0xC0, OPERAND_IMMEDIATE), // Return from subroutine
	INSTRUCTION_1_OP("LDM", 0xD0, OPERAND_IMMEDIATE), // Load memory content into accumulator
	INSTRUCTION_0_OP("CLB", 0xF0), // Clear accumulator and carry
	INSTRUCTION_0_OP("CLC", 0xF1), // Clear carry only
	INSTRUCTION_0_OP("IAC", 0xF2), // Increment accumulator
	INSTRUCTION_0_OP("CMC", 0xF3), // Complement carry
	INSTRUCTION_0_OP("CMA", 0xF4), // Complement accumulator
	INSTRUCTION_0_OP("RAL", 0xF5), //
	INSTRUCTION_0_OP("RAR", 0xF6), //
	INSTRUCTION_0_OP("TCC", 0xF7), // Transfer carry to accumulator
	INSTRUCTION_0_OP("DAC", 0xF8), // Decrement accumulator
	INSTRUCTION_0_OP("TCS", 0xF9), //
	INSTRUCTION_0_OP("STC", 0xFA), // Set carry
	INSTRUCTION_0_OP("DAA", 0xFB), // Decimal adjust after addition
	INSTRUCTION_0_OP("KBP", 0xFC),
	INSTRUCTION_0_OP("DCL", 0xFD),
	INSTRUCTION_0_OP("WRM", 0xE0), // Write to RAM output
	INSTRUCTION_0_OP("WMP", 0xE1), // Write to RAM output port
	INSTRUCTION_0_OP("WRR", 0xE2), // Write to ROM register
	INSTRUCTION_0_OP("WPM", 0xE3), // Write to program port
	INSTRUCTION_0_OP("WR0", 0xE4),
	INSTRUCTION_0_OP("WR1", 0xE5),
	INSTRUCTION_0_OP("WR2", 0xE6),
	INSTRUCTION_0_OP("WR3", 0xE7),
	INSTRUCTION_0_OP("SBM", 0xE8), // Subtraction with memory content
	INSTRUCTION_0_OP("RDM", 0xE9), // Read from RAM
	INSTRUCTION_0_OP("RDR", 0xEA), // Read from ROM register
	INSTRUCTION_0_OP("ADM", 0xEB), // Addition with memory content
	INSTRUCTION_0_OP("RD0", 0xEC),
	INSTRUCTION_0_OP("RD1", 0xED),
	INSTRUCTION_0_OP("RD2", 0xEE),
	INSTRUCTION_0_OP("RD3", 0xEF)
};

#define instruction_count (sizeof(instructions) / sizeof(instructions[0]))

// Prototypes
operand_info get_operand_info(operand type);
const instruction* find_instruction(const char *mnemonic);
