#pragma once

#include <stdlib.h>

#if defined(__GNUC__) || defined(__clang__)
#	if defined(__cplusplus) && (__cplusplus > 199711L)
#		define LOOM_array_alloca(arr, realsize) (decltype(arr))alloca(sizeof(*arr) * (realsize))
#	else
#		define LOOM_array_alloca(arr, realsize) (typeof(arr))alloca(sizeof(*arr) * (realsize))
#	endif
#else
#	define LOOM_array_alloca(arr, realsize) alloca(sizeof(*arr) * (realsize))
#endif
