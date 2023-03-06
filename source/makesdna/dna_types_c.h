#pragma once

/**
 *  Altough we have cumstom defined enums for common types in here
 * more advanced and module specific types may have their own non-compatible
 * enums, for example with the current defines in here we are not able
 * to defined VEC_2_10_10_10I, VEC_10_10_10_2I or VEC_2_10_10_10U,
 * VEC_10_10_10_2U so GPU's shader module may have different enums for these
 * types.
 *
 *  We should altough try to keep as much defines as possible in sync with these
 * here for the sake of consistency. Enums here only take 3 bytes
 */

typedef unsigned long LOOM_DWORD;
typedef int LOOM_BOOL;
typedef unsigned char LOOM_BYTE;
typedef unsigned short LOOM_WORD;
typedef LOOM_DWORD *LOOM_LPDWORD;
typedef LOOM_WORD *LOOM_LPWORD;
typedef int LOOM_LONG;

typedef int LOOM_INT;
typedef unsigned int LOOM_UINT;
typedef unsigned int *LOOM_PUINT;

#define LOOM_8U 0
#define LOOM_8S 1
#define LOOM_10S 2
#define LOOM_16U 3
#define LOOM_32U 4
#define LOOM_32S 5
#define LOOM_32F 6
#define LOOM_16F 7

#define LOOM_CN_MAX 255
#define LOOM_CN_SHIFT 4
#define LOOM_DEPTH_MAX (1 << LOOM_CN_SHIFT)

#define LOOM_MAT_DEPTH_MASK (LOOM_DEPTH_MAX - 1)
#define LOOM_MAT_DEPTH(flags) ((flags)&LOOM_MAT_DEPTH_MASK)

#define LOOM_MAKEDIM(a, b) \
	(((LOOM_WORD)((a)&0x0f) << 4) | ((LOOM_WORD)((b)&0x0f)))
#define LOOM_LODIM(l) (((LOOM_BYTE)((l) >> 4) & 0x0f))
#define LOOM_HIDIM(l) ((LOOM_BYTE)((l)&0x0f))
#define LOOM_DIM_SHIFT 8

#define LOOM_MAKETYPE(depth, rows, cols) \
	(LOOM_MAT_DEPTH(depth) | ((LOOM_MAKEDIM(rows, cols)) << LOOM_CN_SHIFT))
#define LOOM_TYPE_DIM(type) ((type) >> LOOM_CN_SHIFT)
#define LOOM_CHANNELS(type) \
	(LOOM_LODIM(LOOM_TYPE_DIM(type)) * LOOM_HIDIM(LOOM_TYPE_DIM(type)))
#define LOOM_ROWS(type) (LOOM_LODIM(LOOM_TYPE_DIM(type)))
#define LOOM_COLS(type) (LOOM_HIDIM(LOOM_TYPE_DIM(type)))

#define LOOM_DEPTH_BITS(type) \
	size_t(((0x10202020100A0808ULL) >> (LOOM_MAT_DEPTH(type) << 3)) & 0xff)
#define LOOM_SIZEOF(type) \
	size_t((LOOM_DEPTH_BITS(type) * LOOM_CHANNELS(type) + 7) >> 3)

#define LOOM_TYPE_SHIFT (LOOM_DIM_SHIFT + LOOM_CN_SHIFT)
#define LOOM_TYPE_MAX (1 << LOOM_CN_SHIFT)
#define LOOM_TYPE_MASK (LOOM_TYPE_MAX - 1)

#define LOOM_MAGIC_MASK (0xffff & (~LOOM_TYPE_MASK))

/**
 * Create a special type enum that has extra user-defined magic data to specify
 * extra proprties of the specified type, e.g. byte order. The 'magic' keyword
 * can be anywhere between 0 and 15 but it can be changed to store a short
 * instead if needed but then a type enum will not be able to be stored in a
 * short, now the distribution of bytes is : [4b magic][4b row][4b col][4b
 * depth] and therefore the total is 16bytes which can be stored in an unsigned
 * short altough we will usually store them in unsigned int variables.
 */
#define LOOM_MAKETYPE_EX(depth, rows, cols, magic) \
	(LOOM_MAKETYPE(depth, rows, cols) | ((magic) << LOOM_TYPE_SHIFT))
#define LOOM_MAGIC(type) (((type) >> LOOM_TYPE_SHIFT) & 0x0f)
