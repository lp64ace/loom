#pragma once

#include "loomlib_endian_defines.h"

/* -------------------------------------------------------------------- */
/* \name Function Argument Utils
 * \{ */

#if defined(_MSC_VER) && (_MSC_VER > 1910)
#	define ATTR_PRINTF_FORMAT_STRING _In_z_ _Printf_format_string_
#else
#	define ATTR_PRINTF_FORMAT_STRING
#endif

/** \} */

/* -------------------------------------------------------------------- */
/** \name Misc Macros
 * \{ */

#define EXPR_NOP(expr) (void)(0 ? ((void)(expr), 1) : 0)

/**
 * Utility macro that wraps `std::enable_if` to make it a bit easier to use and
 * less verbose for SFINAE in common cases.
 *
 * \note Often one has to invoke this macro with double parenthesis. That's
 * because the condition often contains a comma and angle brackets are not
 * recognized as parenthesis by the preprocessor.
 */
#define LOOM_ENABLE_IF(condition) \
	typename std::enable_if_t<(condition)> * = nullptr

#if defined(_MSC_VER)
#	define LOOM_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#elif defined(__has_cpp_attribute)
#	if __has_cpp_attribute(no_unique_address)
#		define LOOM_NO_UNIQUE_ADDRESS [[no_unique_address]]
#	else
#		define LOOM_NO_UNIQUE_ADDRESS
#	endif
#else
#	define LOOM_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

/** \} */

/* -------------------------------------------------------------------- */
/* \name Inline Macro
 * \{ */

#if defined(_MSC_VER)
#	define LOOM_INLINE static __inline
#else
#	define LOOM_INLINE static inline
#endif

/** \} */
