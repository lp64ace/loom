#pragma once

#include "guardedalloc/mem_guardedalloc.h"

#include "loomlib_utildefines.h"
#include "loomlib_compiler.h"

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

/**
 * Duplicate a string and return the new buffer.
 * \param string The byte-string we wish to duplicate.
 * \return Returns the newly allocated buffer containing the 
 * string duplicate, deallocation should be done using #MEM_freeN.
 */
char *GLU_strdup ( const char *string );

/**
 * Duplicate a string and return the new buffer.
 * \param string The byte-string we wish to duplicate.
 * \param n The number of charachters we want to copy.
 * \return Returns the newly allocated buffer containing the
 * string duplicate, deallocation should be done using #MEM_freeN.
 */
char *GLU_strdupn ( const char *string , size_t n );

/**
 * Appends two string, and returns new mallocN'ed string
 * \param str1: first string for copy.
 * \param str2: second string to append.
 * \return Returns the newly allocated string.
 */
char *GLU_strdupcat ( const char *__restrict str1 , const char *__restrict str2 );

/**
 * Return the length of the null-terminated string STR. Scan for
 * the null terminator quickly by testing four bytes at a time.
 */
size_t GLU_strlen ( const char *string );

/**
 * Return the length of the fixed-size string STR. Scan for
 * the null terminator quickly by testing four bytes at a time.
 */
size_t GLU_strnlen ( const char *string , size_t maxn );

/**
 * Like strncpy but ensures dst is always '\0' terminated.
 * 
 * \param dst: The destination buffer to copy the source string at.
 * \param src: The source string we want to copy.
 * \param maxncpy: Maximum number of charachters to copy (generally the size 
 * of \a dst ).
 * \return Retursn the destination buffer string.
 */
char *GLU_strncpy ( char *__restrict dst , const char *__restrict src , size_t maxncpy );

#ifdef __cplusplus
}
#endif
