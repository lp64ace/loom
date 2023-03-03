#pragma once

#include "guardedalloc/mem_guardedalloc.h"

#include "loomlib_compiler_typecheck.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Warning : The Bitmap does not keep track of its own size or check for 
 * out-of-bounds bit access.
 */
typedef unsigned int LoomBitmap;

#define _BITMAP_POWER 5
#define _BITMAP_MASK 31

/**
 * The number of blocks needed to hold '_num' bits
 */
#define _BITMAP_NUM_BLOCKS(_num) \
  (((_num) + _BITMAP_MASK) >> _BITMAP_POWER)

/**
 * Size (in bytes) used to hold '_num' bits
 */
#define LOOM_BITMAP_SIZE(_num) \
  ((size_t)(_BITMAP_NUM_BLOCKS(_num)) * sizeof(LoomBitmap))

/**
 * Allocate memory for a bitmap with '_num' bits; free with MEM_free().
 */
#define LOOM_BITMAP_NEW(_num, _alloc_string) \
  ((LoomBitmap *)MEM_callocN(LOOM_BITMAP_SIZE(_num), _alloc_string))

/**
 * Allocate a bitmap on the stack.
 */
#define LOOM_BITMAP_NEW_ALLOCA(_num) \
  ((LoomBitmap *)memset(alloca(LOOM_BITMAP_SIZE(_num)), 0, LOOM_BITMAP_SIZE(_num)))

/**
 * Declares a bitmap as a variable.
 */
#define LOOM_BITMAP_DECLARE(_name, _num) \
  LoomBitmap _name[_BITMAP_NUM_BLOCKS(_num)] = {}

#define LOOM_BITMAP_TEST(_bitmap, _index) \
  ((LoomBitmap)[(_index) >> _BITMAP_POWER] & (1u << ((_index)&_BITMAP_MASK)))

#define LOOM_BITMAP_TEST_BOOL(_bitmap, _index) \
  (LOOM_BITMAP_TEST(_bitmap, _index) != 0)

/**
 * Set the value of a single bit at '_index'.
 */
#define LOOM_BITMAP_ENABLE(_bitmap, _index) \
  ((LoomBitmap)[(_index) >> _BITMAP_POWER] |= (1u << ((_index)&_BITMAP_MASK)))

/**
 * Clear the value of a single bit at '_index'.
 */
#define LOOM_BITMAP_DISABLE(_bitmap, _index) \
  ((LoomBitmap)[(_index) >> _BITMAP_POWER] &= ~(1u << ((_index)&_BITMAP_MASK)))

/**
 * Flip the value of a single bit at '_index'.
 */
#define LOOM_BITMAP_FLIP(_bitmap, _index) \
  ((LoomBitmap)[(_index) >> _BITMAP_POWER] ^= (1u << ((_index)&_BITMAP_MASK)))

/** 
 * Macro to set/unset specified bit in bitmap
 */
#define LOOM_BITMAP_SET(_bitmap, _index, _set) \
	{ \
		if (_set) { \
			LOOM_BITMAP_ENABLE(_bitmap, _index); \
		} \
		else { \
			LOOM_BITMAP_DISABLE(_bitmap, _index); \
		} \
	} \
	(void)0

/**
 * Macro to resize the number of bits we can store in the bitmap
 */
#define LOOM_BITMAP_RESIZE(_bitmap, _num) \
	{ \
		(_bitmap) = MEM_recallocN(_bitmap, LOOM_BITMAP_SIZE(_num)); \
	} \
	(void)0

#ifdef __cplusplus
}
#endif
