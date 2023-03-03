#include "../loomlib_utildefines.h"
#include "../loomlib_hash.h"

unsigned int GLU_hash_string_u32(const char *str)
{
	unsigned int value = 0;
	for ( const char *c = str; *c; c++) {
		value = value * 97 + (*c);
	}
	return value;
}
