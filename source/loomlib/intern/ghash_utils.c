#include "guardedalloc/mem_guardedalloc.h"

#include "loomlib/loomlib_ghash.h"
#include "loomlib/loomlib_hash_mm2a.h"
#include "loomlib/loomlib_utildefines.h"

#include <string.h>

/* -------------------------------------------------------------------- */
/** \name Generic Key Hash & Comparison Functions
 * \{ */

unsigned int GLU_ghashutil_ptrhash(const void *key)
{
	/* Based Python3.7's pointer hashing function. */

	unsigned int y = (unsigned int)key;
	/* bottom 3 or 4 bits are likely to be 0; rotate y by 4 to avoid
	 * excessive hash collisions for dictionaries and sets */

	/* NOTE: Unlike Python `sizeof(uint)` is used instead of `sizeof(void *)`,
	 * Otherwise casting to 'uint' ignores the upper bits on 64bit platforms. */
	return (unsigned int)(y >> 4) |
		   ((unsigned int)y << (sizeof(unsigned int[8]) - 4));
}

bool GLU_ghashutil_ptrcmp(const void *a, const void *b)
{
	return (a != b);
}

unsigned int GLU_ghashutil_uinthash_v4(const unsigned int key[4])
{
	unsigned int hash = 0;
	for (const unsigned int *itr = key; itr != key + 4; itr++) {
		hash = hash * 37 + *itr;
	}
	return hash;
}

unsigned int GLU_ghashutil_uinthash_v4_murmur(const unsigned int key[4])
{
	return GLU_hash_mm2(
		(const unsigned char *)key, sizeof(int[4]) /* sizeof(key) */, 0);
}

bool GLU_ghashutil_uinthash_v4_cmp(const void *a, const void *b)
{
	return (memcmp(a, b, sizeof(unsigned int[4])) != 0);
}

unsigned int GLU_ghashutil_uinthash(unsigned int key)
{
	key += ~(key << 16);
	key ^= (key >> 5);
	key += (key << 3);
	key ^= (key >> 13);
	key += ~(key << 9);
	key ^= (key >> 17);

	return key;
}

unsigned int GLU_ghashutil_inthash_p(const void *ptr)
{
	uintptr_t key = (uintptr_t)ptr;

	key += ~(key << 16);
	key ^= (key >> 5);
	key += (key << 3);
	key ^= (key >> 13);
	key += ~(key << 9);
	key ^= (key >> 17);

	return (unsigned int)(key & 0xffffffff);
}

unsigned int GLU_ghashutil_inthash_p_murmur(const void *ptr)
{
	uintptr_t key = (uintptr_t)ptr;

	return GLU_hash_mm2((const unsigned char *)&key, sizeof(key), 0);
}

unsigned int GLU_ghashutil_inthash_p_simple(const void *ptr)
{
	return POINTER_AS_UINT(ptr);
}

bool GLU_ghashutil_intcmp(const void *a, const void *b)
{
	return (a != b);
}

unsigned int GLU_ghashutil_combine_hash(unsigned int hash_a,
										unsigned int hash_b)
{
	return hash_a ^ (hash_b + 0x9e3779b9 + (hash_a << 6) + (hash_a >> 2));
}

unsigned int GLU_ghashutil_strhash_n(const char *key, unsigned int n)
{
	const signed char *p;
	unsigned int h = 5381;

	for (p = (const signed char *)key; n-- && *p != '\0'; p++) {
		h = (unsigned int)((h << 5) + h) + (unsigned int)*p;
	}

	return h;
}

unsigned int GLU_ghashutil_strhash_p(const void *ptr)
{
	const signed char *p;
	unsigned int h = 5381;

	for (p = ptr; *p != '\0'; p++) {
		h = (unsigned int)((h << 5) + h) + (unsigned int)*p;
	}

	return h;
}

unsigned int GLU_ghashutil_strhash_p_murmur(const void *ptr)
{
	const unsigned char *key = ptr;

	return GLU_hash_mm2(key, strlen((const char *)key) + 1, 0);
}

bool GLU_ghashutil_strcmp(const void *a, const void *b)
{
	return (a == b) ? false : !STREQ(a, b);
}

GHashPair *GLU_ghashutil_pairalloc(const void *first, const void *second)
{
	GHashPair *pair = MEM_mallocN(sizeof(GHashPair), "GHashPair");
	pair->first = first;
	pair->second = second;
	return pair;
}

unsigned int GLU_ghashutil_pairhash(const void *ptr)
{
	const GHashPair *pair = ptr;
	unsigned int hash = GLU_ghashutil_ptrhash(pair->first);
	return hash ^ GLU_ghashutil_ptrhash(pair->second);
}

bool GLU_ghashutil_paircmp(const void *a, const void *b)
{
	const GHashPair *A = a;
	const GHashPair *B = b;

	return ((A->first != B->first) || (A->second != B->second));
}

void GLU_ghashutil_pairfree(void *ptr)
{
	MEM_freeN(ptr);
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Convenience GHash Creation Functions
 * \{ */

GHash *GLU_ghash_ptr_new_ex(const char *info,
							const unsigned int nentries_reserve)
{
	return GLU_ghash_new_ex(
		GLU_ghashutil_ptrhash, GLU_ghashutil_ptrcmp, info, nentries_reserve);
}

GHash *GLU_ghash_ptr_new(const char *info)
{
	return GLU_ghash_ptr_new_ex(info, 0);
}

GHash *GLU_ghash_str_new_ex(const char *info,
							const unsigned int nentries_reserve)
{
	return GLU_ghash_new_ex(
		GLU_ghashutil_strhash_p, GLU_ghashutil_strcmp, info, nentries_reserve);
}

GHash *GLU_ghash_str_new(const char *info)
{
	return GLU_ghash_str_new_ex(info, 0);
}

GHash *GLU_ghash_int_new_ex(const char *info,
							const unsigned int nentries_reserve)
{
	return GLU_ghash_new_ex(
		GLU_ghashutil_inthash_p, GLU_ghashutil_intcmp, info, nentries_reserve);
}

GHash *GLU_ghash_int_new(const char *info)
{
	return GLU_ghash_int_new_ex(info, 0);
}

GHash *GLU_ghash_pair_new_ex(const char *info,
							 const unsigned int nentries_reserve)
{
	return GLU_ghash_new_ex(
		GLU_ghashutil_pairhash, GLU_ghashutil_paircmp, info, nentries_reserve);
}

GHash *GLU_ghash_pair_new(const char *info)
{
	return GLU_ghash_pair_new_ex(info, 0);
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Convenience GSet Creation Functions
 * \{ */

GSet *GLU_gset_ptr_new_ex(const char *info, const unsigned int nentries_reserve)
{
	return GLU_gset_new_ex(
		GLU_ghashutil_ptrhash, GLU_ghashutil_ptrcmp, info, nentries_reserve);
}

GSet *GLU_gset_ptr_new(const char *info)
{
	return GLU_gset_ptr_new_ex(info, 0);
}

GSet *GLU_gset_str_new_ex(const char *info, const unsigned int nentries_reserve)
{
	return GLU_gset_new_ex(
		GLU_ghashutil_strhash_p, GLU_ghashutil_strcmp, info, nentries_reserve);
}

GSet *GLU_gset_str_new(const char *info)
{
	return GLU_gset_str_new_ex(info, 0);
}

GSet *GLU_gset_pair_new_ex(const char *info,
						   const unsigned int nentries_reserve)
{
	return GLU_gset_new_ex(
		GLU_ghashutil_pairhash, GLU_ghashutil_paircmp, info, nentries_reserve);
}

GSet *GLU_gset_pair_new(const char *info)
{
	return GLU_gset_pair_new_ex(info, 0);
}

GSet *GLU_gset_int_new_ex(const char *info, const unsigned int nentries_reserve)
{
	return GLU_gset_new_ex(
		GLU_ghashutil_inthash_p, GLU_ghashutil_intcmp, info, nentries_reserve);
}

GSet *GLU_gset_int_new(const char *info)
{
	return GLU_gset_int_new_ex(info, 0);
}

/** \} */