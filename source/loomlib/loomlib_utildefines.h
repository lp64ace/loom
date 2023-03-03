#pragma once

#include "loomlib_compiler.h"
#include "loomlib_compiler_typecheck.h"
#include "loomlib_sys_types.h"

#include <string.h>
#include <float.h>

/* -------------------------------------------------------------------- */
/* \name Array Memory Management
 * \{ */

#define ARRAY_SIZE(a)		(sizeof(a)/sizeof((a)[0]))

#define UNPACK2(a)		((a)[0]), ((a)[1])
#define UNPACK3(a)		((a)[0]), ((a)[1]), ((a)[2])
#define UNPACK4(a)		((a)[0]), ((a)[1]), ((a)[2]), ((a)[3])

#define UNPACK2_EX(pre, a, post)	(pre)((a)[0]), (pre)((a)[1])post
#define UNPACK3_EX(pre, a, post)	UNPACK2_EX(pre, a, post), (pre)((a)[2])post
#define UNPACK4_EX(pre, a, post)	UNPACK3_EX(pre, a, post), (pre)((a)[3])post

/** \} */

/* -------------------------------------------------------------------- */
/** \name Array Helpers
 * \{ */

#define ARRAY_LAST_ITEM(arr_start, arr_dtype, arr_len) \
  (arr_dtype *)((char *)(arr_start) + (sizeof(*((arr_dtype *)NULL)) * (size_t)(arr_len - 1)))

#define ARRAY_HAS_ITEM(arr_item, arr_start, arr_len) \
  (((ptrdiff_t)((arr_item) - (arr_start)) < (size_t)(arr_len)))

 /** \} */

/* -------------------------------------------------------------------- */
/** \name Pointer Macros
 * \{ */

#if defined(__GNUC__) || defined(__clang__)
#  define POINTER_OFFSET(v, ofs) ((typeof(v))((char *)(v) + (ofs)))
#else
#  define POINTER_OFFSET(v, ofs) ((void *)((char *)(v) + (ofs)))
#endif

#define POINTER_FROM_INT(i) ((void *)(intptr_t)(i))
#define POINTER_AS_INT(i) ((int)(intptr_t)(i))

#define POINTER_FROM_UINT(i) ((void *)(uintptr_t)(i))
#define POINTER_AS_UINT(i) ((unsigned int)(uintptr_t)(i))

/** \} */

/* -------------------------------------------------------------------- */
/* \name Swap/Shift Macros
 * \{ */

#define SWAP(type, a, b) \
  { \
    type sw_ap; \
    CHECK_TYPE(a, type); \
    CHECK_TYPE(b, type); \
    sw_ap = (a); \
    (a) = (b); \
    (b) = sw_ap; \
  } \
  (void)0

#define SWAP_TVAL(tval, a, b) \
  { \
    CHECK_TYPE_PAIR(tval, a); \
    CHECK_TYPE_PAIR(tval, b); \
    (tval) = (a); \
    (a) = (b); \
    (b) = sw_ap; \
  } \
  (void)0

#define SHIFT3(type, a, b, c) \
  { \
    type tmp; \
    CHECK_TYPE(a, type); \
    CHECK_TYPE(b, type); \
    CHECK_TYPE(c, type); \
    tmp = (a); \
    (a) = (c); \
    (c) = (b); \
    (b) = tmp; \
  } \
  (void)0

#define SHIFT4(type, a, b, c, d) \
  { \
    type tmp; \
    CHECK_TYPE(a, type); \
    CHECK_TYPE(b, type); \
    CHECK_TYPE(c, type); \
    CHECK_TYPE(d, type); \
    tmp = (a); \
    (a) = (d); \
    (d) = (c); \
    (c) = (b); \
    (b) = tmp; \
  } \
  (void)0

/** \} */

/* -------------------------------------------------------------------- */
/* \name Min/Max Macros
 * \{ */

#if !defined(NDEBUG)
#  define _TYPECHECK(a, b) ((void)(((typeof(a) *)0) == ((typeof(b) *)0)))
#  define MIN2(x, y) (_TYPECHECK(x, y), (((x) < (y)) ? (x) : (y)))
#  define MAX2(x, y) (_TYPECHECK(x, y), (((x) > (y)) ? (x) : (y)))
#else
#  define MIN2(x, y) (((x) < (y)) ? (x) : (y))
#  define MAX2(x, y) (((x) > (y)) ? (x) : (y))
#endif

#define MIN3(a, b, c) (MIN2(MIN2(a, b), c))
#define MIN4(a, b, c, d) (MIN2(MIN2(a, b), MIN2(c, d)))

#define MAX3(a, b, c) (MAX2(MAX2(a, b), c))
#define MAX4(a, b, c, d) (MAX2(MAX2(a, b), MAX2(c, d)))

/** \} */

/* -------------------------------------------------------------------- */
/** \name Equal to Any Element (ELEM) Macro
 * \{ */

#define _VA_ELEM2(v, a) \
  ((v) == (a))
#define _VA_ELEM3(v, a, b) \
  (_VA_ELEM2(v, a) || _VA_ELEM2(v, b))
#define _VA_ELEM4(v, a, b, c) \
  (_VA_ELEM3(v, a, b) || _VA_ELEM2(v, c))
#define _VA_ELEM5(v, a, b, c, d) \
  (_VA_ELEM4(v, a, b, c) || _VA_ELEM2(v, d))
#define _VA_ELEM6(v, a, b, c, d, e) \
  (_VA_ELEM5(v, a, b, c, d) || _VA_ELEM2(v, e))
#define _VA_ELEM7(v, a, b, c, d, e, f) \
  (_VA_ELEM6(v, a, b, c, d, e) || _VA_ELEM2(v, f))
#define _VA_ELEM8(v, a, b, c, d, e, f, g) \
  (_VA_ELEM7(v, a, b, c, d, e, f) || _VA_ELEM2(v, g))
#define _VA_ELEM9(v, a, b, c, d, e, f, g, h) \
  (_VA_ELEM8(v, a, b, c, d, e, f, g) || _VA_ELEM2(v, h))
#define _VA_ELEM10(v, a, b, c, d, e, f, g, h, i) \
  (_VA_ELEM9(v, a, b, c, d, e, f, g, h) || _VA_ELEM2(v, i))
#define _VA_ELEM11(v, a, b, c, d, e, f, g, h, i, j) \
  (_VA_ELEM10(v, a, b, c, d, e, f, g, h, i) || _VA_ELEM2(v, j))
#define _VA_ELEM12(v, a, b, c, d, e, f, g, h, i, j, k) \
  (_VA_ELEM11(v, a, b, c, d, e, f, g, h, i, j) || _VA_ELEM2(v, k))
#define _VA_ELEM13(v, a, b, c, d, e, f, g, h, i, j, k, l) \
  (_VA_ELEM12(v, a, b, c, d, e, f, g, h, i, j, k) || _VA_ELEM2(v, l))
#define _VA_ELEM14(v, a, b, c, d, e, f, g, h, i, j, k, l, m) \
  (_VA_ELEM13(v, a, b, c, d, e, f, g, h, i, j, k, l) || _VA_ELEM2(v, m))
#define _VA_ELEM15(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n) \
  (_VA_ELEM14(v, a, b, c, d, e, f, g, h, i, j, k, l, m) || _VA_ELEM2(v, n))
#define _VA_ELEM16(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) \
  (_VA_ELEM15(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n) || _VA_ELEM2(v, o))
#define _VA_ELEM17(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) \
  (_VA_ELEM16(v, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) || _VA_ELEM2(v, p))

// Checks if the first argument equal to any of the remaining arguments
#define ELEM(...) VA_NARGS_CALL_OVERLOAD(_VA_ELEM, __VA_ARGS__)

/** \} */

/* -------------------------------------------------------------------- */
/** \name Simple Math Macros
 * \{ */

#define IS_EQ(a, b) \
  (CHECK_TYPE_INLINE(a, double), \
   CHECK_TYPE_INLINE(b, double), \
   ((fabs((double)((a)-(b))) >= (double)FLT_EPSILON) ? false : true))

#define IS_EQF(a, b) \
  (CHECK_TYPE_INLINE(a, float), \
   CHECK_TYPE_INLINE(b, float), \
   ((fabs((float)((a)-(b))) >= (float)FLT_EPSILON) ? false : true))

#define IS_EQT(a, b, c) (((a) > (b)) ? ((((a) - (b)) <= (c))) : (((((b) - (a)) <= (c)))))
#define IN_RANGE(a, b, c) (((b) < (c)) ? (((b) < (a) && (a) < (c))) : (((c) < (a) && (a) < (b))))
#define IN_RANGE_INCL(a, b, c) (((b) < (c)) ? (((b) <= (a) && (a) <= (c))) : (((c) <= (a) && (a) <= (b))))

/**
* Expands to an integer constant expression evaluating to a close upper bound
* on the number the number of decimal digits in a value expressible in the
* integer type given by the argument (if it is a type name) or the integer
* type of the argument (if it is an expression). The meaning of the resulting
* expression is unspecified for other arguments.
* i.e: `DECIMAL_DIGITS_BOUND(uchar)` is equal to 3.
*/
#define DECIMAL_DIGITS_BOUND(t) (241 * sizeof(t) / 100 + 1)

/** \} */

/* -------------------------------------------------------------------- */
/* \name Clamp Macros
 * \{ */

#define CLAMPIS(a, b, c) ((a) < (b) ? (b) : (a) > (c) ? (c) : (a))

#define CLAMP(a, b, c) \
  { \
    if ((a) < (b)) { \
      (a) = (b); \
    } \
    else if ((a) > (c)) { \
      (a) = (c); \
    } \
  } \
  (void)0

#define CLAMP_MAX(a, c) \
  { \
    if ((a) > (c)) { \
      (a) = (c); \
    } \
  } \
  (void)0

#define CLAMP_MIN(a, b) \
  { \
    if ((a) < (b)) { \
      (a) = (b); \
    } \
  } \
  (void)0

#define CLAMP2(vec, b, c) \
  { \
    CLAMP((vec)[0], b, c); \
    CLAMP((vec)[1], b, c); \
  } \
  (void)0

#define CLAMP2_MIN(vec, b) \
  { \
    CLAMP_MIN((vec)[0], b); \
    CLAMP_MIN((vec)[1], b); \
  } \
  (void)0

#define CLAMP2_MAX(vec, c) \
  { \
    CLAMP_MAX((vec)[0], c); \
    CLAMP_MAX((vec)[1], c); \
  } \
  (void)0

#define CLAMP3(vec, b, c) \
  { \
    CLAMP((vec)[0], b, c); \
    CLAMP((vec)[1], b, c); \
    CLAMP((vec)[2], b, c); \
  } \
  (void)0

#define CLAMP3_MIN(vec, b) \
  { \
    CLAMP_MIN((vec)[0], b); \
    CLAMP_MIN((vec)[1], b); \
    CLAMP_MIN((vec)[2], b); \
  } \
  (void)0

#define CLAMP3_MAX(vec, c) \
  { \
    CLAMP_MAX((vec)[0], c); \
    CLAMP_MAX((vec)[1], c); \
    CLAMP_MAX((vec)[2], c); \
  } \
  (void)0

#define CLAMP4(vec, b, c) \
  { \
    CLAMP((vec)[0], b, c); \
    CLAMP((vec)[1], b, c); \
    CLAMP((vec)[2], b, c); \
    CLAMP((vec)[3], b, c); \
  } \
  (void)0

#define CLAMP4_MIN(vec, b) \
  { \
    CLAMP_MIN((vec)[0], b); \
    CLAMP_MIN((vec)[1], b); \
    CLAMP_MIN((vec)[2], b); \
    CLAMP_MIN((vec)[3], b); \
  } \
  (void)0

#define CLAMP4_MAX(vec, c) \
  { \
    CLAMP_MAX((vec)[0], c); \
    CLAMP_MAX((vec)[1], c); \
    CLAMP_MAX((vec)[2], c); \
    CLAMP_MAX((vec)[3], c); \
  } \
  (void)0

/** \} */

/* -------------------------------------------------------------------- */
/* \name String Macros
 * \{ */

#define STRINGIFY_ARG(x) "" #x
#define STRINGIFY_APPEND(a, b) "" a #b
#define STRINGIFY(x) STRINGIFY_APPEND("", x)

#if defined(_MSC_VER)
#  define strcasecmp _stricmp
#  define strncasecmp _strnicmp
#endif

#define STREQ(a, b) (strcmp(a, b) == 0)
#define STRCASEEQ(a, b) (strcasecmp(a, b) == 0)
#define STREQLEN(a, b, n) (strncmp(a, b, n) == 0)
#define STRCASEEQLEN(a, b, n) (strncasecmp(a, b, n) == 0)

#define STRPREFIX(a, b) (strncmp((a), (b), strlen(b)) == 0)

/** \} */

/* -------------------------------------------------------------------- */
/* \name Unused Function/Arguments Macros
 * \{ */

#ifndef __cplusplus
#  if defined(__GNUC__) || defined(__clang__)
#    define UNUSED(x) UNUSED_##x __attribute__((unused))
#  else
#    define UNUSED(x) UNUSED_##x
#  endif
#endif

/**
 * WARNING: this doesn't warn when returning pointer types (because of the placement of `*`).
 * Use #UNUSED_FUNCTION_WITH_RETURN_TYPE instead in this case.
 */
#if defined(__GNUC__) || defined(__clang__)
#  define UNUSED_FUNCTION(x) __attribute__((__unused__)) UNUSED_##x
#else
#  define UNUSED_FUNCTION(x) UNUSED_##x
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define UNUSED_FUNCTION_WITH_RETURN_TYPE(rtype, x) __attribute__((__unused__)) rtype UNUSED_##x
#else
#  define UNUSED_FUNCTION_WITH_RETURN_TYPE(rtype, x) rtype UNUSED_##x
#endif

/** \} */

/* -------------------------------------------------------------------- */
/* \name Branch Prediction Macros
 * \{ */

#if defined(__GNUC__)
#  define LIKELY(x) __builtin_expect(!!(x), 1)
#  define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#  define LIKELY(x) (x)
#  define UNLIKELY(x) (x)
#endif

/** \} */

/* -------------------------------------------------------------------- */
/* \name Flag Macros
 * \{ */

#define SET_FLAG_FROM_TEST(value, test, flag) \
  { \
    if (test) { \
      (value) |= (flag); \
    } else { \
      (value) &= ~(flag); \
    } \
  } \
  (void) 0

/** \} */

/* -------------------------------------------------------------------- */
/** \name Enum operators
 * \{ */

#ifdef __cplusplus
 // Useful to port C code using enums to C++ where enums are strongly typed.
 // To use after the enum declaration.
 // If any enumerator `C` is set to say `A|B`, then `C` would be the max enum value.
#  define ENUM_OPERATORS(_enum_type, _max_enum_value) \
	extern "C++" { \
		inline constexpr _enum_type operator|(_enum_type a, _enum_type b) { \
			return static_cast<_enum_type>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b)); \
		} \
		inline constexpr _enum_type operator&(_enum_type a, _enum_type b) { \
			return static_cast<_enum_type>(static_cast<uint64_t>(a) & static_cast<uint64_t>(b)); \
		} \
		inline constexpr _enum_type operator~(_enum_type a) { \
			return static_cast<_enum_type>(~static_cast<uint64_t>(a) & (2 * static_cast<uint64_t>(_max_enum_value) - 1)); \
		} \
		inline _enum_type &operator|=(_enum_type &a, _enum_type b) { \
			return a = static_cast<_enum_type>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b)); \
		} \
		inline _enum_type &operator&=(_enum_type &a, _enum_type b) { \
			return a = static_cast<_enum_type>(static_cast<uint64_t>(a) & static_cast<uint64_t>(b)); \
		} \
	} /* extern "C++" */
#else
#  define ENUM_OPERATORS(_type, _max)
#endif

#ifdef __cplusplus
#  define CPP_ARG_DEFAULT(default_value) = default_value
#else
#  define CPP_ARG_DEFAULT(default_value)
#endif

/** \} */
