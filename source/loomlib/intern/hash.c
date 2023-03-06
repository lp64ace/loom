#include "loomlib/loomlib_hash.h"
#include "loomlib/loomlib_utildefines.h"

unsigned int GLU_hash_string_u32(const char *str)
{
	unsigned int value = 0;
	for (const char *c = str; *c; c++) {
		value = value * 97 + (*c);
	}
	return value;
}
