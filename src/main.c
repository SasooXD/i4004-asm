#include <stdio.h>
#include <stdlib.h>

#include "assembler.h"

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <source.asm>\n", argv[0]);
		return 1;
	}

	const char *src = argv[1];

	// Build output filename
	char *out = replace_extension(src, ".bin");
	if (!out)
	{
		fprintf(stderr, "Error: could not allocate output filename\n");
		return 1;
	}

	// Pass 1: collect labels
	symbol_t symbols[MAX_SYMBOLS];
	size_t   sym_count = 0;
	pass1_collect_labels(src, symbols, &sym_count);

	// Pass 2: assemble with label resolution
	int err = pass2_assemble(src, out, symbols, sym_count);

	free(out);
	return err;
}
