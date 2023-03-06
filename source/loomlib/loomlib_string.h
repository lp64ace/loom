#pragma once

#include "guardedalloc/mem_guardedalloc.h"

#include "loomlib_compiler.h"
#include "loomlib_config.h"
#include "loomlib_utildefines.h"

#include <stdarg.h>

/**
 * The reason this file even exists and we are not using the builtin
 * C or C++ string methods, is because we are using MEM_guardedalloc to
 * allocate and deallocate dynamic strings. That said, strings that
 * are duplicated using #strdup cannot be freed using #MEM_freeN which is
 * a dissaster as it may cause segmentation faults, while at it we implement
 * function that do not use memory allocations or deallocations as well for
 * the sake of consistency in the code.
 *
 * \note The perfomance of #GLU_strlen is yet to be tested but it is expected to
 * perform better than #strlen in most cases as it uses a faster compare
 * method than #strlen.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------- */
/** \name String Duplicate/Copy
 * \{ */

/**
 * Duplicate a string and return the new buffer.
 * \param string The byte-string we wish to duplicate.
 * \return Returns the newly allocated buffer containing the
 * string duplicate, deallocation should be done using #MEM_freeN.
 */
char *GLU_strdup(const char *string);

/**
 * Duplicate a string and return the new buffer.
 * \param string The byte-string we wish to duplicate.
 * \param n The number of charachters we want to copy.
 * \return Returns the newly allocated buffer containing the
 * string duplicate, deallocation should be done using #MEM_freeN.
 */
char *GLU_strdupn(const char *string, size_t n);

/**
 * Appends two string, and returns new mallocN'ed string
 * \param str1: first string for copy.
 * \param str2: second string to append.
 * \return Returns the newly allocated string.
 */
char *GLU_strdupcat(const char *__restrict str1, const char *__restrict str2);

/**
 * Return the length of the null-terminated string STR. Scan for
 * the null terminator quickly by testing four bytes at a time.
 */
size_t GLU_strlen(const char *string);

/**
 * Return the length of the fixed-size string STR. Scan for
 * the null terminator quickly by testing four bytes at a time.
 */
size_t GLU_strnlen(const char *string, size_t maxn);

/**
 * Like strncpy but ensures dst is always '\0' terminated.
 *
 * \param dst: The destination buffer to copy the source string at.
 * \param src: The source string we want to copy.
 * \param maxncpy: Maximum number of charachters to copy (generally the size
 * of \a dst ).
 * \return Retursn the destination buffer string.
 */
char *GLU_strncpy(char *__restrict dst,
				  const char *__restrict src,
				  size_t maxncpy);

/**
 * Like #GLU_strncpy but ensures dst is always paddded by given char,
 * on both sides (unless src is empty).
 *
 * \param dst: Destination buffer to copy the source string at.
 * \param src: The source string we want to copy.
 * \param pad: The char to use for padding.
 * \param maxncpy: Maximum number of charachter to copy (generally the size of
 * dst) \retval Returns the destination buffer.
 */
char *GLU_strncpy_ensure_pad(char *__restrict dst,
							 const char *__restrict src,
							 char pad,
							 size_t maxncpy);

/**
 * Like #GLU_strncpy but ensures dst is always paddded by given char,
 * on both sides (unless src is empty).
 *
 * \param dst: Destination buffer to copy the source string at.
 * \param src: The source string we want to copy.
 * \param pad: The char to use for padding.
 * \param maxncpy: Maximum number of charachter to copy (generally the size of
 * dst) \retval The number of bytes copied (The only difference from
 * GLU_strncpy).
 */
size_t GLU_strncpy_rlen(char *__restrict dst,
						const char *__restrict src,
						const size_t maxncpy);

/**
 * Same as #GLU_strncpy but it copies the full src string without any checks of
 * wether there is enough space. Use with caution. \param dst: The destination
 * buffer to receive the string. \param src: The source string we want to copy.
 * \retval The number of bytes that were copied.
 */
size_t GLU_strcpy_rlen(char *__restrict dst, const char *__restrict src);

/** \} */

/* -------------------------------------------------------------------- */
/** \name String Transform
 * \{ */

/**
 * All instances of \a substr_old are replaced by \a substr_new,
 * Returns a copy of the c-string \a str into a newly #MEM_mallocN'd
 * and returns it.
 *
 * \note A rather wasteful string-replacement utility, though this shall do for
 * now. Feel free to replace this with an even safe + nicer alternative. \remark
 * Time complexity should be O ( |str| + |substr_old| ) to detect the duplicates
 * and O ( |str| ) <= actual_complexity <= O ( |str| * |substr_new| /
 * |substr_old| ) to create the new string. It is safe to assume that the
 * complexity will never exceed : O ( |str| * |substr_new| + |substr_old| )!
 *
 * \param str: The string to replace occurrences of substr_old in.
 * \param substr_old: The text in the string to find and replace.
 * \param substr_new: The string to replace the found string with.
 * \retval Returns the duplicated string.
 */
char *GLU_str_replaceN(const char *__restrict str,
					   const char *__restrict substr_old,
					   const char *__restrict substr_new);

/**
 * In-place replace every occurrence of \a src to \a dst in \a str.
 *
 * \param std: The string to operate on.
 * \param src: The charachter to replace.
 * \param dst: The charachter to replace with.
 * \retval Returns \a str.
 */
char *GLU_str_replace_char(char *str, const char src, const char dst);

/**
 * In-place reverse string.
 */
char *GLU_str_reverse(char *str);

/**
 * Reverses the string and stores it in a newly allocated buffer.
 * The buffer must be freed with #MEM_freeN.
 * \param str The string we want to reverse.
 * \retval The newly allocated string with the bytes of the source string in
 * reverse orders.
 */
char *GLU_str_reverseN(const char *__restrict str);

/** \} */

/* -------------------------------------------------------------------- */
/* \name Print String
 * \{ */

/**
 * Portable replacement for #snprintf
 */
size_t GLU_snprintf(char *__restrict dst,
					size_t maxncpy,
					const char *ATTR_PRINTF_FORMAT_STRING format,
					...);

/**
 * A version of #GLU_snprintf taht returns `GLU_strlen(dst)`.
 */
size_t GLU_snprintf_rlen(char *__restrict dst,
						 size_t maxncpy,
						 const char *ATTR_PRINTF_FORMAT_STRING format,
						 ...);

/**
 * Portable replacement for `vsnprintf`.
 */
size_t GLU_vsnprintf(char *__restrict buffer,
					 size_t maxncpy,
					 const char *ATTR_PRINTF_FORMAT_STRING format,
					 va_list arg);

/**
 * A version of #GLU_vsnprintf that returns `GLU_strlen(buffer)`
 */
size_t GLU_vsnprintf_rlen(char *__restrict buffer,
						  size_t maxncpy,
						  const char *ATTR_PRINTF_FORMAT_STRING format,
						  va_list arg);

/**
 * Print formatted string into a newly #MEM_mallocN'd string and return it.
 */
char *GLU_sprintfN(const char *__restrict format, ...);

/** \} */

#ifdef __cplusplus
}
#endif
