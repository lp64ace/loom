#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------- */
/* \name Utility functions
 * \{ */

void _LOOM_assert_print_pos(const char *file,
							int line,
							const char *function,
							const char *id);
void _LOOM_assert_print_extra(const char *str);
void _LOOM_assert_print_backtrace(void);
void _LOOM_assert_abort(void);
void _LOOM_assert_unreachable_print(const char *file,
									int line,
									const char *function);

/** \} */

#ifdef _MSC_VER
#	include <crtdbg.h>
#endif

#if !defined(NDEBUG)

#	if defined(__GNUC__)
#		define _LOOM_ASSERT_PRINT_POS(a) \
			_LOOM_assert_print_pos(__FILE__, __LINE__, __func__, #a)
#	elif defined(_MSC_VER)
#		define _LOOM_ASSERT_PRINT_POS(a) \
			_LOOM_assert_print_pos(__FILE__, __LINE__, __func__, #a)
#	else
#		define _LOOM_ASSERT_PRINT_POS(a) \
			_LOOM_assert_print_pos(__FILE__, __LINE__, "<?>", #a)
#	endif

#	ifdef _DEBUG
#		define _LOOM_ASSERT_ABORT _LOOM_assert_abort
#	else
#		define _LOOM_ASSERT_ABORT() (void)0
#	endif

#	define LOOM_assert(a) \
		(void)((!(a)) ? ((_LOOM_assert_print_backtrace(), \
						  _LOOM_ASSERT_PRINT_POS(a), \
						  _LOOM_ASSERT_ABORT(), \
						  NULL)) : \
						NULL)
/** A version of #BLI_assert() to pass an additional message to be printed on
 * failure. */
#	define LOOM_assert_msg(a, msg) \
		(void)((!(a)) ? ((_LOOM_assert_print_backtrace(), \
						  _LOOM_ASSERT_PRINT_POS(a), \
						  _LOOM_assert_print_extra(msg), \
						  _LOOM_ASSERT_ABORT(), \
						  NULL)) : \
						NULL)
#else
#	define LOOM_assert(a) ((void)0)
#	define LOOM_assert_msg(a, msg) ((void)0)
#endif

/* -------------------------------------------------------------------- */
/* \name Static Assert
 * \{ */

#if defined(__cplusplus)
#	define LOOM_STATIC_ASSERT(a, msg) static_assert(a, msg); /* C++11 */
#elif defined(_MSC_VER)
#	if (_MSC_VER > 1910) && !(defined(__clang__)) /* Visual Studio */
#		define LOOM_STATIC_ASSERT(a, msg) static_assert(a, msg);
#	else
#		define LOOM_STATIC_ASSERT(a, msg) _STATIC_ASSERT(a);
#	endif
#elif defined(__COVERITY__)
#	define LOOM_STATIC_ASSERT(a, msg) \
		((void)0) /* Workaround error with COVERITY */
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#	define LOOM_STATIC_ASSERT(a, msg) _Static_assert(a, msg); /* C11 */
#else
#	define LOOM_STATIC_ASSERT(a, msg) /* Old unsupported compiler */
#endif

/** \} */

#define LOOM_STATIC_ASSERT_ALIGN(st, align) \
	LOOM_STATIC_ASSERT((sizeof(st) % (align) == 0), \
					   "Structure must be strictly aligned")

/**
 * Indicates that this line of code should never be executed. If it is reached,
 * it will abort in debug builds and print an error in release builds.
 */
#define LOOM_assert_unreachable() \
	{ \
		_LOOM_assert_unreachable_print(__FILE__, __LINE__, __func__); \
		LOOM_assert_msg(0, "This line of code is marked to be unreachable."); \
	} \
	((void)0)

#ifdef __cplusplus
}
#endif
