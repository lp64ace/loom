#pragma once

#include "mallocn_inline.h"

#define SIZET_FORMAT "%zu"
#define SIZET_ARG(a) ((size_t)(a))

#define SIZET_ALIGN_4(len) ((len + 3) & ~(size_t)3)

#if !defined(__APPLE__) && !defined(__FreeBSD__) && !defined(__NetBSD__)
// Needed for memalign on Linux and _aligned_alloc on Windows.
#	include <malloc.h>
#else
// Apple's malloc is 16-byte aligned, and does not have malloc.h, so include
// stdilb instead.
#	include <stdlib.h>
#endif

#define IS_POW2(a) (((a) & ((a)-1)) == 0)

/* Extra padding which needs to be applied on MemHead to make it aligned. */
#define MEMHEAD_ALIGN_PADDING(alignment) \
	((size_t)alignment - (sizeof(MemHeadAligned) % (size_t)alignment))

/* Real pointer returned by the malloc or aligned_alloc. */
#define MEMHEAD_REAL_PTR(memh) \
	((char *)memh - MEMHEAD_ALIGN_PADDING(memh->alignment))

#ifdef __cplusplus
extern "C" {
#endif

#define ALIGNED_MALLOC_MINIMUM_ALIGNMENT sizeof(void *)

void *aligned_malloc(size_t size, size_t alignment);
void aligned_free(void *ptr);

extern bool leak_detector_has_run;
extern char leak_detector_has_run_msg[];

void memory_usage_init(void);
void memory_usage_block_alloc(size_t size);
void memory_usage_block_free(size_t size);
size_t memory_usage_block_num(void);
size_t memory_usage_current(void);
size_t memory_usage_peak(void);
void memory_usage_peak_reset(void);

/* -------------------------------------------------------------------- */
/* \name Prototypes for counted allocator functions
 * \{ */

size_t MEM_lockfree_allocN_len(const void *vmemh);
void MEM_lockfree_freeN(void *vmemh);
void *MEM_lockfree_dupallocN(const void *vmemh);
void *MEM_lockfree_reallocN_id(void *vmemh, size_t len, const char *str);
void *MEM_lockfree_recallocN_id(void *vmemh, size_t len, const char *str);
void *MEM_lockfree_callocN(size_t len, const char *str);
void *MEM_lockfree_calloc_arrayN(size_t len, size_t size, const char *str);
void *MEM_lockfree_mallocN(size_t len, const char *str);
void *MEM_lockfree_malloc_arrayN(size_t len, size_t size, const char *str);
void *MEM_lockfree_mallocN_aligned(size_t len,
								   size_t alignment,
								   const char *str);
void MEM_lockfree_printmemlist(void);
void MEM_lockfree_callbackmemlist(void (*func)(void *));
void MEM_lockfree_printmemlist_stats(void);
void MEM_lockfree_set_error_callback(void (*func)(const char *));
bool MEM_lockfree_consistency_check(void);
void MEM_lockfree_set_memory_debug(void);
size_t MEM_lockfree_get_memory_in_use(void);
size_t MEM_lockfree_get_memory_blocks_in_use(void);
void MEM_lockfree_reset_peak_memory(void);
size_t MEM_lockfree_get_peak_memory(void);

#ifndef NDEBUG
const char *MEM_lockfree_name_ptr(void *vmemh);
void MEM_lockfree_name_ptr_set(void *vmemh, const char *str);
#endif

/** \} */

/* -------------------------------------------------------------------- */
/* \name Prototypes for fully guarded allocator functions
 * \{ */

size_t MEM_guarded_allocN_len(const void *vmemh);
void MEM_guarded_freeN(void *vmemh);
void *MEM_guarded_dupallocN(const void *vmemh);
void *MEM_guarded_reallocN_id(void *vmemh, size_t len, const char *str);
void *MEM_guarded_recallocN_id(void *vmemh, size_t len, const char *str);
void *MEM_guarded_callocN(size_t len, const char *str);
void *MEM_guarded_calloc_arrayN(size_t len, size_t size, const char *str);
void *MEM_guarded_mallocN(size_t len, const char *str);
void *MEM_guarded_malloc_arrayN(size_t len, size_t size, const char *str);
void *MEM_guarded_mallocN_aligned(size_t len,
								  size_t alignment,
								  const char *str);
void MEM_guarded_printmemlist(void);
void MEM_guarded_callbackmemlist(void (*func)(void *));
void MEM_guarded_printmemlist_stats(void);
void MEM_guarded_set_error_callback(void (*func)(const char *));
bool MEM_guarded_consistency_check(void);
void MEM_guarded_set_memory_debug(void);
size_t MEM_guarded_get_memory_in_use(void);
size_t MEM_guarded_get_memory_blocks_in_use(void);
void MEM_guarded_reset_peak_memory(void);
size_t MEM_guarded_get_peak_memory(void);

#ifndef NDEBUG
const char *MEM_guarded_name_ptr(void *vmemh);
void MEM_guarded_name_ptr_set(void *vmemh, const char *str);
#endif

/** \} */

#ifdef __cplusplus
}
#endif
