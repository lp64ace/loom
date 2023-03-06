#pragma once

/* -------------------------------------------------------------------- */
/** \name Detect Endian
 * \{ */

#define __ORDER_LITTLE_ENDIAN__ 0x524F5345UL
#define __ORDER_BIG_ENDIAN__ 0x45534F52UL
#define __ORDER_PDP_ENDIAN__ 0x4F524553UL

#define __BYTE_ORDER__ ('ROSE')

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#	define __LITTLE_ENDIAN__ 1
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#	define __BIG_ENDIAN__ 1
#endif

/** \} */

/** ENDIAN_ORDER: indicates what endianness the platform where the file was
 * written had. */
#if !defined(__BIG_ENDIAN__) && !defined(__LITTLE_ENDIAN__)
#	error Either __BIG_ENDIAN__ or __LITTLE_ENDIAN__ must be defined.
#endif

#define L_ENDIAN 1
#define B_ENDIAN 0

#ifdef __BIG_ENDIAN__
#	define ENDIAN_ORDER B_ENDIAN
#else
#	define ENDIAN_ORDER L_ENDIAN
#endif
