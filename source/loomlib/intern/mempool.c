#include "guardedalloc/mem_guardedalloc.h"

#include "atomic/atomic_ops.h"

#include "loomlib/loomlib_assert.h"
#include "loomlib/loomlib_compiler.h"
#include "loomlib/loomlib_mempool.h"
#include "loomlib/loomlib_utildefines.h"

#include <stdlib.h>
#include <string.h>

#if defined(__BIG_ENDIAN__) && __BIG_ENDIAN__
/* Big Endian */
#	define MAKE_ID(a, b, c, d) \
		((int)(a) << 24 | (int)(b) << 16 | (c) << 8 | (d))
#	define MAKE_ID_8(a, b, c, d, e, f, g, h) \
		((int64_t)(a) << 56 | (int64_t)(b) << 48 | (int64_t)(c) << 40 | \
		 (int64_t)(d) << 32 | (int64_t)(e) << 24 | (int64_t)(f) << 16 | \
		 (int64_t)(g) << 8 | (h))
#elif defined(__LITTLE_ENDIAN__) && __LITTLE_ENDIAN__
/* Little Endian */
#	define MAKE_ID(a, b, c, d) \
		((int)(d) << 24 | (int)(c) << 16 | (b) << 8 | (a))
#	define MAKE_ID_8(a, b, c, d, e, f, g, h) \
		((int64_t)(h) << 56 | (int64_t)(g) << 48 | (int64_t)(f) << 40 | \
		 (int64_t)(e) << 32 | (int64_t)(d) << 24 | (int64_t)(c) << 16 | \
		 (int64_t)(b) << 8 | (a))
#else
#	error Unspecified endian provided
#endif

/** Important that this value is an is _not_  aligned with `sizeof(void *)`.
 * So having a pointer to 2/4/8... aligned memory is enough to ensure
 * the `freeword` will never be used.
 * To be safe, use a word that's the same in both directions. */
#define FREEWORD \
	((sizeof(void *) > sizeof(int32_t)) ? \
		 MAKE_ID_8('e', 'e', 'r', 'f', 'f', 'r', 'e', 'e') : \
		 MAKE_ID('e', 'f', 'f', 'e'))

// The 'used' word just needs to be set to something besides FREEWORD.
#define USEDWORD MAKE_ID('u', 's', 'e', 'd')

typedef struct FreeNode {
	struct FreeNode *next;
	intptr_t freeword;
} FreeNode;

typedef struct MemPoolChunk {
	struct MemPoolChunk *next;
} MemPoolChunk;

struct MemPool {
	MemPoolChunk *chunks;
	MemPoolChunk *chunk_tail;

	size_t esize;
	size_t csize;
	size_t pchunk;

	int flag;

	FreeNode *free;

	size_t maxchunks;
	size_t totused;
};

#define MEMPOOL_ELEM_SIZE_MIN (sizeof(void *) * 2)

#define CHUNK_DATA(chunk) ((void *)((chunk) + 1))

#define NODE_STEP_NEXT(node) ((void *)((char *)(node) + esize))
#define NODE_STEP_PREV(node) ((void *)((char *)(node)-esize))

#define CHUNK_OVERHEAD (size_t)(MEM_SIZE_OVERHEAD + sizeof(MemPoolChunk))

static size_t power_of_2_max_u(size_t x)
{
	x -= 1;
	x = x | (x >> 1);
	x = x | (x >> 2);
	x = x | (x >> 4);
	x = x | (x >> 8);
	x = x | (x >> 16);
	return x + 1;
}

LOOM_INLINE MemPoolChunk *mempool_chunk_find(MemPoolChunk *head, size_t index)
{
	while (index-- && head) {
		head = head->next;
	}
	return head;
}

LOOM_INLINE size_t mempool_maxchunks(const size_t nelem, const size_t pchunk)
{
	return (nelem <= pchunk) ? 1 : ((nelem / pchunk) + 1);
}

static MemPoolChunk *mempool_chunk_alloc(MemPool *pool)
{
	return MEM_mallocN(sizeof(MemPoolChunk) + (size_t)pool->csize, __func__);
}

static FreeNode *mempool_chunk_add(MemPool *pool,
								   MemPoolChunk *mpchunk,
								   FreeNode *last_tail)
{
	const size_t esize = pool->esize;
	FreeNode *curnode = CHUNK_DATA(mpchunk);
	size_t j;

	/* append */
	if (pool->chunk_tail) {
		pool->chunk_tail->next = mpchunk;
	}
	else {
		LOOM_assert(pool->chunks == NULL);
		pool->chunks = mpchunk;
	}

	mpchunk->next = NULL;
	pool->chunk_tail = mpchunk;

	if (UNLIKELY(pool->free == NULL)) {
		pool->free = curnode;
	}

	/* loop through the allocated data, building the pointer structures */
	j = pool->pchunk;
	if (pool->flag & LOOM_MEMPOOL_ALLOW_ITER) {
		while (j--) {
			curnode->next = NODE_STEP_NEXT(curnode);
			curnode->freeword = FREEWORD;
			curnode = curnode->next;
		}
	}
	else {
		while (j--) {
			curnode->next = NODE_STEP_NEXT(curnode);
			curnode = curnode->next;
		}
	}

	/* terminate the list (rewind one)
	 * will be overwritten if 'curnode' gets passed in again as 'last_tail' */
	curnode = NODE_STEP_PREV(curnode);
	curnode->next = NULL;

	/* final pointer in the previously allocated chunk is wrong */
	if (last_tail) {
		last_tail->next = CHUNK_DATA(mpchunk);
	}

	return curnode;
}

static void mempool_chunk_free(MemPoolChunk *mpchunk)
{
	MEM_freeN(mpchunk);
}

static void mempool_chunk_free_all(MemPoolChunk *mpchunk)
{
	MemPoolChunk *mpchunk_next = NULL;

	for (; mpchunk; mpchunk = mpchunk_next) {
		mpchunk_next = mpchunk->next;
		mempool_chunk_free(mpchunk);
	}
}

MemPool *GLU_mempool_create(size_t elem_size,
							size_t elem_num,
							size_t per_chunk,
							int flag)
{
	MemPool *pool = NULL;
	FreeNode *tail = NULL;

	size_t i;

	pool = MEM_mallocN(sizeof(MemPool), __func__);

	if (elem_size < (size_t)MEMPOOL_ELEM_SIZE_MIN) {
		elem_size = (size_t)MEMPOOL_ELEM_SIZE_MIN;
	}

	if (flag & LOOM_MEMPOOL_ALLOW_ITER) {
		elem_size = MAX2(elem_size, (size_t)sizeof(FreeNode));
	}

	pool->chunks = NULL;
	pool->chunk_tail = NULL;

	pool->esize = elem_size;
	{
		LOOM_assert(power_of_2_max_u(per_chunk * elem_size) > CHUNK_OVERHEAD);
		per_chunk = (power_of_2_max_u(per_chunk * elem_size) - CHUNK_OVERHEAD) /
					elem_size;
	}
	pool->csize = elem_size * per_chunk;
	{
		size_t final_size = (size_t)MEM_SIZE_OVERHEAD +
							(size_t)sizeof(MemPoolChunk) + pool->csize;
		LOOM_assert(((size_t)power_of_2_max_u(final_size) - final_size) <
					pool->esize);
	}
	pool->pchunk = per_chunk;

	pool->flag = flag;
	pool->free = NULL;

	pool->maxchunks = mempool_maxchunks(elem_num, per_chunk);

	if (elem_num) {
		/* Allocate the actual chunks. */
		for (i = 0; i < pool->maxchunks; i++) {
			MemPoolChunk *mpchunk = mempool_chunk_alloc(pool);
			tail = mempool_chunk_add(pool, mpchunk, tail);
		}
	}

	return pool;
}

void *GLU_mempool_alloc(MemPool *pool)
{
	FreeNode *free_pop;

	if (UNLIKELY(pool->free == NULL)) {
		/* Need to allocate a new chunk. */
		MemPoolChunk *mpchunk = mempool_chunk_alloc(pool);
		mempool_chunk_add(pool, mpchunk, NULL);
	}

	free_pop = pool->free;

	LOOM_assert(pool->chunk_tail->next == NULL);

	if (pool->flag & LOOM_MEMPOOL_ALLOW_ITER) {
		free_pop->freeword = USEDWORD;
	}

	pool->free = free_pop->next;
	pool->totused++;

	return (void *)free_pop;
}

void *GLU_mempool_calloc(MemPool *pool)
{
	void *retval = GLU_mempool_alloc(pool);
	memset(retval, 0, (size_t)pool->esize);
	return retval;
}

void GLU_mempool_free(MemPool *pool, void *addr)
{
	FreeNode *newhead = addr;

#ifndef NDEBUG
	{
		MemPoolChunk *chunk;
		bool found = false;
		for (chunk = pool->chunks; chunk; chunk = chunk->next) {
			if (ARRAY_HAS_ITEM(
					(char *)addr, (char *)CHUNK_DATA(chunk), pool->csize)) {
				found = true;
				break;
			}
		}
		if (!found) {
			LOOM_assert_msg(0, "Attempt to free data which is not in pool.\n");
		}
	}

	memset(addr, 0xcd, pool->esize);
#endif

	if (pool->flag & LOOM_MEMPOOL_ALLOW_ITER) {
#ifndef NDEBUG
		LOOM_assert(newhead->freeword != FREEWORD);
#endif
		newhead->freeword = FREEWORD;
	}

	newhead->next = pool->free;
	pool->free = newhead;

	pool->totused--;

	/* Nothing is in use; free all the chunks except the first. */
	if (UNLIKELY(pool->totused == 0) && (pool->chunks->next)) {
		const size_t esize = pool->esize;
		FreeNode *curnode;
		size_t j;
		MemPoolChunk *first;

		first = pool->chunks;
		mempool_chunk_free_all(first->next);
		first->next = NULL;
		pool->chunk_tail = first;

		curnode = CHUNK_DATA(first);
		pool->free = curnode;

		j = pool->pchunk;
		while (j--) {
			curnode->next = NODE_STEP_NEXT(curnode);
			curnode = curnode->next;
		}
		curnode = NODE_STEP_PREV(curnode);
		curnode->next = NULL;
	}
}

size_t GLU_mempool_len(const MemPool *pool)
{
	return pool->totused;
}

void *GLU_mempool_findelem(MemPool *pool, size_t index)
{
	LOOM_assert(pool->flag & LOOM_MEMPOOL_ALLOW_ITER);

	if (index < pool->totused) {
		/* We could have some faster mem chunk stepping code inline. */
		MemPoolIter iter;
		void *elem;
		GLU_mempool_iternew(pool, &iter);
		for (elem = GLU_mempool_iterstep(&iter); index-- != 0;
			 elem = GLU_mempool_iterstep(&iter)) {
		}
		return elem;
	}

	return NULL;
}

void GLU_mempool_iternew(MemPool *pool, MemPoolIter *iter)
{
	LOOM_assert(pool->flag & LOOM_MEMPOOL_ALLOW_ITER);

	iter->pool = pool;
	iter->chunk = pool->chunks;
	iter->index = 0;
}

void *GLU_mempool_iterstep(MemPoolIter *iter)
{
	if (UNLIKELY(iter->chunk == NULL)) {
		return NULL;
	}

	const size_t esize = iter->pool->esize;
	FreeNode *curnode = POINTER_OFFSET(CHUNK_DATA(iter->chunk),
									   (esize * iter->index));
	FreeNode *ret;
	do {
		ret = curnode;

		if (++iter->index != iter->pool->pchunk) {
			curnode = POINTER_OFFSET(curnode, esize);
		}
		else {
			iter->index = 0;
			iter->chunk = iter->chunk->next;
			if (UNLIKELY(iter->chunk == NULL)) {
				return (ret->freeword == FREEWORD) ? NULL : ret;
			}
			curnode = CHUNK_DATA(iter->chunk);
		}
	} while (ret->freeword == FREEWORD);

	return ret;
}

void GLU_mempool_clear_ex(MemPool *pool, size_t totelem_reserve)
{
	MemPoolChunk *mpchunk;
	MemPoolChunk *mpchunk_next;
	size_t maxchunks;

	MemPoolChunk *chunks_temp;
	FreeNode *last_tail = NULL;

	if (totelem_reserve == -1) {
		maxchunks = pool->maxchunks;
	}
	else {
		maxchunks = mempool_maxchunks((size_t)totelem_reserve, pool->pchunk);
	}

	/* Free all after 'pool->maxchunks'. */
	mpchunk = mempool_chunk_find(pool->chunks, maxchunks - 1);
	if (mpchunk && mpchunk->next) {
		/* terminate */
		mpchunk_next = mpchunk->next;
		mpchunk->next = NULL;
		mpchunk = mpchunk_next;

		do {
			mpchunk_next = mpchunk->next;
			mempool_chunk_free(mpchunk);
		} while ((mpchunk = mpchunk_next));
	}

	/* re-initialize */
	pool->free = NULL;
	pool->totused = 0;

	chunks_temp = pool->chunks;
	pool->chunks = NULL;
	pool->chunk_tail = NULL;

	while ((mpchunk = chunks_temp)) {
		chunks_temp = mpchunk->next;
		last_tail = mempool_chunk_add(pool, mpchunk, last_tail);
	}
}

void GLU_mempool_clear(MemPool *pool)
{
	GLU_mempool_clear_ex(pool, -1);
}

void GLU_mempool_discard(MemPool *pool)
{
	mempool_chunk_free_all(pool->chunks);

	MEM_freeN(pool);
}
