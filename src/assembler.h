#include <stdint.h>
#include <stddef.h>

// Maximum number of symbols
#define MAX_SYMBOLS 256

// Maximum line length for source lines
#define MAX_LINE_LENGTH 256

// Maximum number of tokens per line (mnemonic + operands)
#define MAX_TOKENS 3

// Symbol table entry: a label name and its address
typedef struct {
    char     name[32];   // Label identifier (null-terminated)
    uint16_t address;    // Address (PC) where label is defined
} symbol_t;

// Pass 1: scan source, collect label definitions into 'table'
//   filename: path to .asm file
//   table:    array to fill with symbols
//   count:    out parameter set to number of labels found
void pass1_collect_labels(const char *filename,
                           symbol_t     table[],
                           size_t      *count);

// Pass 2: assemble source into binary, resolving labels
//   infile:       path to .asm file
//   outfile:      path to .bin output file
//   table:        symbol table from pass1
//   symbol_count: number of entries in 'table'
// Returns 0 on success, non-zero on error
int pass2_assemble(const char *infile,
                   const char *outfile,
                   const symbol_t table[],
                   size_t        symbol_count);

// Parses a string operand into a numeric value.
// Supports register names like R1, register pairs like R0R1, and numeric values.
// Returns -1 on error.
int16_t parse_operand_value(const char *str);

// Assembles a single instruction based on its mnemonic and operand strings.
// Writes the result into the 'output' buffer (1 or 2 bytes).
// Returns the instruction size (1 or 2), or -1 on error.
int8_t assemble_instruction(const char *mnemonic, const char *operands[], uint8_t output[2]);

// Removes comments (starting with ';') and trims whitespace from a line.
void strip_comment_and_whitespace(char *line);

// Splits a line into tokens (mnemonic and operands).
// Returns the number of tokens (max MAX_TOKENS).
int tokenize_line(char *line, char *tokens[MAX_TOKENS]);

// Utility function: returns a newly allocated string with a changed extension.
// Example: ("program.asm", ".bin") → "program.bin"
// The caller must free the returned string.
char *replace_extension(const char *filename, const char *new_ext);

