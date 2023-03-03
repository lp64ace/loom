#pragma once

#include "loomlib_compiler.h"
#include "loomlib_utildefines_variadic.h"

#define TYPE_MISMATCH_ERROR "Variable Type Mismatch"

/* -------------------------------------------------------------------- */
/* \name Check Type of Given Variable
 * \{ */

#if defined(__cplusplus)
#  define CHECK_TYPE(var, type) \
     { \
       LOOM_STATIC_ASSERT(std::is_same<decltype(var), type>::value, TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#  define CHECK_TYPE(var, type) \
     { \
       LOOM_STATIC_ASSERT(_Generic((var), type: 1, default: 0), TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#else
#  define CHECK_TYPE(var, type) \
     { \
       EXPR_NOP(var) \
     } \
     (void)0
#endif

#if defined(__cplusplus)
#  define CHECK_TYPE_PAIR(var_a, var_b) \
     { \
       LOOM_STATIC_ASSERT(std::is_same<decltype(var_a), decltype(var_b)>::value, TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#  define CHECK_TYPE_PAIR(var_a, var_b) \
     { \
       LOOM_STATIC_ASSERT(_Generic((var_a), decltype(var_b): 1, default: 0), TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#else
#  define CHECK_TYPE_PAIR(var_a, var_b) \
     { \
       EXPR_NOP(var_a) \
       EXPR_NOP(var_b) \
     } \
     (void)0
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#  define CHECK_TYPE_INLINE(val, type) \
    (void)((void)(((type)0) != (0 ? (val) : ((type)0))), _Generic((val), type : 0, const type : 0))
#else
#  define CHECK_TYPE_INLINE(val, type) ((void)(((type)0) != (0 ? (val) : ((type)0))))
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#  define CHECK_TYPE_PAIR_INLINE(var_a, var_b) \
     (void)((void)(((decltype(var_a))0) != (0 ? (var_b) : ((decltype(var_a))0))), \
  	   _Generic((var_a), decltype(var_b) : 0, const decltype(var_b) : 0))
#else
#  define CHECK_TYPE_PAIR_INLINE(var_a, var_b) (EXPR_NOP(var_a), EXPR_NOP(var_b))
#endif

#if defined(__cplusplus)
#  define _VA_CHECK_TYPE_ANY2(v,a0) \
     { \
       LOOM_STATIC_ASSERT(std::is_same<decltype(v), a0>::value, TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY3(v,a0,a1) \
     { \
       LOOM_STATIC_ASSERT(std::is_same<decltype(v), a0>::value || \
                          std::is_same<decltype(v), a1>::value, TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY4(v,a0,a1,a2) \
     { \
       LOOM_STATIC_ASSERT(std::is_same<decltype(v), a0>::value || \
                          std::is_same<decltype(v), a1>::value || \
                          std::is_same<decltype(v), a2>::value, TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY5(v,a0,a1,a2,a3) \
     { \
       LOOM_STATIC_ASSERT(std::is_same<decltype(v), a0>::value || \
                          std::is_same<decltype(v), a1>::value || \
                          std::is_same<decltype(v), a2>::value || \
                          std::is_same<decltype(v), a3>::value, TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY6(v,a0,a1,a2,a3,a4) \
     { \
       LOOM_STATIC_ASSERT(std::is_same<decltype(v), a0>::value || \
                          std::is_same<decltype(v), a1>::value || \
                          std::is_same<decltype(v), a2>::value || \
                          std::is_same<decltype(v), a3>::value || \
                          std::is_same<decltype(v), a4>::value, TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY7(v,a0,a1,a2,a3,a4,a5) \
     { \
       LOOM_STATIC_ASSERT(std::is_same<decltype(v), a0>::value || \
                          std::is_same<decltype(v), a1>::value || \
                          std::is_same<decltype(v), a2>::value || \
                          std::is_same<decltype(v), a3>::value || \
                          std::is_same<decltype(v), a4>::value || \
                          std::is_same<decltype(v), a5>::value, TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY8(v,a0,a1,a2,a3,a4,a5,a6) \
     { \
       LOOM_STATIC_ASSERT(std::is_same<decltype(v), a0>::value || \
                          std::is_same<decltype(v), a1>::value || \
                          std::is_same<decltype(v), a2>::value || \
                          std::is_same<decltype(v), a3>::value || \
                          std::is_same<decltype(v), a4>::value || \
                          std::is_same<decltype(v), a5>::value || \
                          std::is_same<decltype(v), a6>::value, TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY9(v,a0,a1,a2,a3,a4,a5,a6,a7) \
     { \
       LOOM_STATIC_ASSERT(std::is_same<decltype(v), a0>::value || \
                          std::is_same<decltype(v), a1>::value || \
                          std::is_same<decltype(v), a2>::value || \
                          std::is_same<decltype(v), a3>::value || \
                          std::is_same<decltype(v), a4>::value || \
                          std::is_same<decltype(v), a5>::value || \
                          std::is_same<decltype(v), a6>::value || \
                          std::is_same<decltype(v), a7>::value, TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY10(v,a0,a1,a2,a3,a4,a5,a6,a7,a8) \
     { \
       LOOM_STATIC_ASSERT(std::is_same<decltype(v), a0>::value || \
                          std::is_same<decltype(v), a1>::value || \
                          std::is_same<decltype(v), a2>::value || \
                          std::is_same<decltype(v), a3>::value || \
                          std::is_same<decltype(v), a4>::value || \
                          std::is_same<decltype(v), a5>::value || \
                          std::is_same<decltype(v), a6>::value || \
                          std::is_same<decltype(v), a7>::value || \
                          std::is_same<decltype(v), a8>::value, TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY11(v,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9) \
     { \
       LOOM_STATIC_ASSERT(std::is_same<decltype(v), a0>::value || \
                          std::is_same<decltype(v), a1>::value || \
                          std::is_same<decltype(v), a2>::value || \
                          std::is_same<decltype(v), a3>::value || \
                          std::is_same<decltype(v), a4>::value || \
                          std::is_same<decltype(v), a5>::value || \
                          std::is_same<decltype(v), a6>::value || \
                          std::is_same<decltype(v), a7>::value || \
                          std::is_same<decltype(v), a8>::value || \
                          std::is_same<decltype(v), a9>::value, TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY12(v,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10) \
     { \
       LOOM_STATIC_ASSERT(std::is_same<decltype(v), a0>::value || \
                          std::is_same<decltype(v), a1>::value || \
                          std::is_same<decltype(v), a2>::value || \
                          std::is_same<decltype(v), a3>::value || \
                          std::is_same<decltype(v), a4>::value || \
                          std::is_same<decltype(v), a5>::value || \
                          std::is_same<decltype(v), a6>::value || \
                          std::is_same<decltype(v), a7>::value || \
                          std::is_same<decltype(v), a8>::value || \
                          std::is_same<decltype(v), a9>::value || \
                          std::is_same<decltype(v), a10>::value, TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY13(v,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11) \
     { \
       LOOM_STATIC_ASSERT(std::is_same<decltype(v), a0>::value || \
                          std::is_same<decltype(v), a1>::value || \
                          std::is_same<decltype(v), a2>::value || \
                          std::is_same<decltype(v), a3>::value || \
                          std::is_same<decltype(v), a4>::value || \
                          std::is_same<decltype(v), a5>::value || \
                          std::is_same<decltype(v), a6>::value || \
                          std::is_same<decltype(v), a7>::value || \
                          std::is_same<decltype(v), a8>::value || \
                          std::is_same<decltype(v), a9>::value || \
                          std::is_same<decltype(v), a10>::value || \
                          std::is_same<decltype(v), a11>::value, TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY14(v,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12) \
     { \
       LOOM_STATIC_ASSERT(std::is_same<decltype(v), a0>::value || \
                          std::is_same<decltype(v), a1>::value || \
                          std::is_same<decltype(v), a2>::value || \
                          std::is_same<decltype(v), a3>::value || \
                          std::is_same<decltype(v), a4>::value || \
                          std::is_same<decltype(v), a5>::value || \
                          std::is_same<decltype(v), a6>::value || \
                          std::is_same<decltype(v), a7>::value || \
                          std::is_same<decltype(v), a8>::value || \
                          std::is_same<decltype(v), a9>::value || \
                          std::is_same<decltype(v), a10>::value || \
                          std::is_same<decltype(v), a11>::value || \
                          std::is_same<decltype(v), a12>::value, TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY15(v,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13) \
     { \
       LOOM_STATIC_ASSERT(std::is_same<decltype(v), a0>::value || \
                          std::is_same<decltype(v), a1>::value || \
                          std::is_same<decltype(v), a2>::value || \
                          std::is_same<decltype(v), a3>::value || \
                          std::is_same<decltype(v), a4>::value || \
                          std::is_same<decltype(v), a5>::value || \
                          std::is_same<decltype(v), a6>::value || \
                          std::is_same<decltype(v), a7>::value || \
                          std::is_same<decltype(v), a8>::value || \
                          std::is_same<decltype(v), a9>::value || \
                          std::is_same<decltype(v), a10>::value || \
                          std::is_same<decltype(v), a11>::value || \
                          std::is_same<decltype(v), a12>::value || \
                          std::is_same<decltype(v), a13>::value, TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY16(v,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14) \
     { \
       LOOM_STATIC_ASSERT(std::is_same<decltype(v), a0>::value || \
                          std::is_same<decltype(v), a1>::value || \
                          std::is_same<decltype(v), a2>::value || \
                          std::is_same<decltype(v), a3>::value || \
                          std::is_same<decltype(v), a4>::value || \
                          std::is_same<decltype(v), a5>::value || \
                          std::is_same<decltype(v), a6>::value || \
                          std::is_same<decltype(v), a7>::value || \
                          std::is_same<decltype(v), a8>::value || \
                          std::is_same<decltype(v), a9>::value || \
                          std::is_same<decltype(v), a10>::value || \
                          std::is_same<decltype(v), a11>::value || \
                          std::is_same<decltype(v), a12>::value || \
                          std::is_same<decltype(v), a13>::value || \
                          std::is_same<decltype(v), a14>::value, TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#  define _VA_CHECK_TYPE_ANY2(v,a0) \
     { \
       LOOM_STATIC_ASSERT(_Generic((v), a0: 1, default: 0), TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY3(v,a0,a1) \
     { \
       LOOM_STATIC_ASSERT(_Generic((v), a0: 1, a1: 2, default: 0), TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY4(v,a0,a1,a2) \
     { \
       LOOM_STATIC_ASSERT(_Generic((v), a0: 1, a1: 2, a2: 3, default: 0), TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY5(v,a0,a1,a2,a3) \
     { \
       LOOM_STATIC_ASSERT(_Generic((v), a0: 1, a1: 2, a2: 3, a3: 4, default: 0), TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY6(v,a0,a1,a2,a3,a4) \
     { \
       LOOM_STATIC_ASSERT(_Generic((v), a0: 1, a1: 2, a2: 3, a3: 4, a4: 5, default: 0), TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY7(v,a0,a1,a2,a3,a4,a5) \
     { \
       LOOM_STATIC_ASSERT(_Generic((v), a0: 1, a1: 2, a2: 3, a3: 4, a4: 5, a5: 6, default: 0), TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY8(v,a0,a1,a2,a3,a4,a5,a6) \
     { \
       LOOM_STATIC_ASSERT(_Generic((v), a0: 1, a1: 2, a2: 3, a3: 4, a4: 5, a5: 6, a6: 7, default: 0), TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY9(v,a0,a1,a2,a3,a4,a5,a6,a7) \
     { \
       LOOM_STATIC_ASSERT(_Generic((v), a0: 1, a1: 2, a2: 3, a3: 4, a4: 5, a5: 6, a6: 7, a7: 8, default: 0), TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY10(v,a0,a1,a2,a3,a4,a5,a6,a7,a8) \
     { \
       LOOM_STATIC_ASSERT(_Generic((v), a0: 1, a1: 2, a2: 3, a3: 4, a4: 5, a5: 6, a6: 7, a7: 8, a8: 9, default: 0), TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY11(v,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9) \
     { \
       LOOM_STATIC_ASSERT(_Generic((v), a0: 1, a1: 2, a2: 3, a3: 4, a4: 5, a5: 6, a6: 7, a7: 8, a8: 9, a9: 10, default: 0), TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY12(v,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10) \
     { \
       LOOM_STATIC_ASSERT(_Generic((v), a0: 1, a1: 2, a2: 3, a3: 4, a4: 5, a5: 6, a6: 7, a7: 8, a8: 9, a9: 10, a10: 11, default: 0), TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY13(v,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11) \
     { \
       LOOM_STATIC_ASSERT(_Generic((v), a0: 1, a1: 2, a2: 3, a3: 4, a4: 5, a5: 6, a6: 7, a7: 8, a8: 9, a9: 10, a10: 11, a11: 12, default: 0), TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY14(v,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12) \
     { \
       LOOM_STATIC_ASSERT(_Generic((v), a0: 1, a1: 2, a2: 3, a3: 4, a4: 5, a5: 6, a6: 7, a7: 8, a8: 9, a9: 10, a10: 11, a11: 12, default: 0), TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY15(v,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13) \
     { \
       LOOM_STATIC_ASSERT(_Generic((v), a0: 1, a1: 2, a2: 3, a3: 4, a4: 5, a5: 6, a6: 7, a7: 8, a8: 9, a9: 10, a10: 11, a11: 12, a12: 13, default: 0), TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#  define _VA_CHECK_TYPE_ANY16(v,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14) \
     { \
       LOOM_STATIC_ASSERT(_Generic((v), a0: 1, a1: 2, a2: 3, a3: 4, a4: 5, a5: 6, a6: 7, a7: 8, a8: 9, a9: 10, a10: 11, a11: 12, a12: 13, a13: 14, default: 0), TYPE_MISMATCH_ERROR) \
     } \
     (void)0
#else
#  define _VA_CHECK_TYPE_ANY2(v,a0) EXPR_NOP(v)
#  define _VA_CHECK_TYPE_ANY3(v,a0,a1) EXPR_NOP(v)
#  define _VA_CHECK_TYPE_ANY4(v,a0,a1,a2) EXPR_NOP(v)
#  define _VA_CHECK_TYPE_ANY5(v,a0,a1,a2,a3) EXPR_NOP(v)
#  define _VA_CHECK_TYPE_ANY6(v,a0,a1,a2,a3,a4) EXPR_NOP(v)
#  define _VA_CHECK_TYPE_ANY7(v,a0,a1,a2,a3,a4,a5) EXPR_NOP(v)
#  define _VA_CHECK_TYPE_ANY8(v,a0,a1,a2,a3,a4,a5,a6) EXPR_NOP(v)
#  define _VA_CHECK_TYPE_ANY9(v,a0,a1,a2,a3,a4,a5,a6,a7) EXPR_NOP(v)
#  define _VA_CHECK_TYPE_ANY10(v,a0,a1,a2,a3,a4,a5,a6,a7,a8) EXPR_NOP(v)
#  define _VA_CHECK_TYPE_ANY11(v,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9) EXPR_NOP(v)
#  define _VA_CHECK_TYPE_ANY12(v,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10) EXPR_NOP(v)
#  define _VA_CHECK_TYPE_ANY13(v,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11) EXPR_NOP(v)
#  define _VA_CHECK_TYPE_ANY14(v,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12) EXPR_NOP(v)
#  define _VA_CHECK_TYPE_ANY15(v,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13) EXPR_NOP(v)
#  define _VA_CHECK_TYPE_ANY16(v,a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14) EXPR_NOP(v)
#endif

#define CHECK_TYPE_ANY(...) VA_NARGS_CALL_OVERLOAD(_VA_CHECK_TYPE_ANY, __VA_ARGS__)

/** \} */
