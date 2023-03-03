#pragma once

#include "loomlib_utildefines.h"
#include "loomlib_compiler.h"

struct HashMurmur2A;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HashMurmur2A {
	uint32_t hash;
	uint32_t tail;
	uint32_t count;
	uint32_t size;
} HashMurMur2A;

void GLU_hash_mm2a_init ( struct HashMurMur2A *mm2 , uint32_t seed );
void GLU_hash_mm2a_add ( struct HashMurMur2A *mm2 , const unsigned char *data , size_t len );
void GLU_hash_mm2a_add_int ( struct HashMurMur2A *mm2 , int data );

uint32_t GLU_hash_mm2a_end ( struct HashMurMur2A *mm2 );

// Non-incremental version, quicker for small keys.
uint32_t GLU_hash_mm2 ( const unsigned char *data , size_t len , uint32_t seed );

#ifdef __cplusplus
}
#endif
