#pragma once

#include "loomlib_utildefines.h"
#include "loomlib_compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

struct MemPool;
struct MemPoolChunk;

typedef struct MemPool MemPool;

typedef enum {
	LOOM_MEMPOOL_NOP = 0,
	/** allow iterating on this mempool.
	 * \note this requires that the first four bytes of the elements
	 * never begin with 'free' (#FREEWORD).
	 * \note order of iteration is only assured to be the
	 * order of allocation when no chunks have been freed. */
	LOOM_MEMPOOL_ALLOW_ITER = (1 << 0),
};

/** Generate a mempool of specific static-size elements.
 * \param elem_size The size of a single element in bytes.
 * \param elem_num The number of elements to allocate space for.
 * \param per_chunk The number of elements to store per chunk.
 * \param flag creation flags of the mempool.
 * \return Returns the mempool that was created with the specified parameters. */
MemPool *GLU_mempool_create ( size_t elem_size , size_t elem_num , size_t per_chunk , int flag );

/** Allocate space within the pool for a single element.
 * \param pool The mempool to allocate memory within.
 * \return Returns a pointer to the memory designated for the element. */
void *GLU_mempool_alloc(MemPool *pool);

/** Allocate space within the pool for a single element and fill the newly allocated space with
 * zeros. \param pool The mempool to allocate memory within. \return Returns a pointer to the
 * memory designated for the element. */
void *GLU_mempool_calloc(MemPool *pool);

/** Free a specified element from the mempool.
 * \note doesn't protect against double frees, take care!
 * \param pool The mempool to free the element from.
 * \param addr The address of memory of the element we want to deallocate. */
void GLU_mempool_free(MemPool *pool, void *addr);

/** Empty the pool, as if it were just created.
 * \param pool The mempool to we want to empty.
 * \param totelem_reserve Optionally reserve how many items should be kept from clearing. */
void GLU_mempool_clear_ex(MemPool *pool, size_t totelem_reserve);

/** Remove all elements from the mempool.
 * \param pool The mempool to we want to empty. */
void GLU_mempool_clear(MemPool *pool);

/** Free the mempool itself (and all elements).
 * \param pool The mempool we want to discard. */
void GLU_mempool_discard(MemPool *pool);

/** Get the number of elements within the pool.
 * \param pool The mempool we want to cound the elements of.
 * \return Return the number of elements currently allocated in the mempool. */
size_t GLU_mempool_len(const MemPool *pool);

/** Find the element in the given position in the mempool.
 * \param pool The mempool to search in.
 * \param index The index of the element we want to get.
 * \return Returns the element in the specified index or NULL if \a index is out of bounds. */
void *GLU_mempool_findelem(MemPool *pool, size_t index);

typedef struct MemPoolIter {
	MemPool *pool;
	struct MemPoolChunk *chunk;
	size_t index;
} MemPoolIter;

void GLU_mempool_iternew(MemPool *pool, MemPoolIter *iter);

void *GLU_mempool_iterstep(MemPoolIter *iter);

#ifdef __cplusplus
}
#endif
