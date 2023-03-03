#include "guardedalloc/mem_guardedalloc.h"

#include "../loomlib_utildefines.h"
#include "../loomlib_mempool.h"

#define GHASH_INTERNAL_API
#include "../loomlib_ghash.h"

#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------- */
/** \name Structs & Constants
 * \{ */

#define GHASH_USE_MODULO_BUCKETS

 // Next prime after `2^n` (skipping 2 & 3)
extern const unsigned int GLU_ghash_hash_sizes [ ];
const unsigned int GLU_ghash_hash_sizes [ ] = {
	5,     11,     17,     37,     67,	131,	 257,	  521,	   1031,     2053,     4099,	 8209,	    16411,     32771,
	65537, 131101, 262147, 524309, 1048583, 2097169, 4194319, 8388617, 16777259, 33554467, 67108879, 134217757, 268435459,
};

#define hashsizes GLU_ghash_hash_sizes

#ifdef GHASH_USE_MODULO_BUCKETS
#	define GHASH_MAX_SIZE 27
LOOM_STATIC_ASSERT ( ARRAY_SIZE ( hashsizes ) == GHASH_MAX_SIZE , "Invalid 'hashsizes' size" );
#else
#	define GHASH_BUCKET_BIT_MIN 2
#	define GHASH_BUCKET_BIT_MAX 28 /* About 268M of buckets... */
#endif

/**
 * \note Max load #GHASH_LIMIT_GROW used to be 3. (pre 2.74).
 * Python uses 0.6666, tommyhashlib even goes down to 0.5.
 * Reducing our from 3 to 0.75 gives huge speedup
 * (about twice quicker pure GHash insertions/lookup,
 * about 25% - 30% quicker 'dynamic-topology' stroke drawing e.g.).
 * Min load #GHASH_LIMIT_SHRINK is a quarter of max load, to avoid resizing to quickly.
 */
#define GHASH_LIMIT_GROW(_nbkt) (((_nbkt)*3) / 4)
#define GHASH_LIMIT_SHRINK(_nbkt) (((_nbkt)*3) / 16)

 /* WARNING! Keep in sync with ugly _gh_Entry in header!!! */
typedef struct Entry {
	struct Entry *next;

	void *key;
} Entry;

typedef struct GHashEntry {
	Entry e;

	void *val;
} GHashEntry;

typedef Entry GSetEntry;

#define GHASH_ENTRY_SIZE(_is_gset) ((_is_gset) ? sizeof(GSetEntry) : sizeof(GHashEntry))

struct GHash {
	GHashHashFP hashfp;
	GHashCmpFP cmpfp;

	Entry **buckets;
	struct MemPool *entrypool;
	unsigned int nbuckets;
	unsigned int limit_grow , limit_shrink;
#ifdef GHASH_USE_MODULO_BUCKETS
	unsigned int cursize , size_min;
#else
	unsigned int bucket_mask , bucket_bit , bucket_bit_min;
#endif

	unsigned int nentries;
	unsigned int flag;
};

/** \} */

/* -------------------------------------------------------------------- */
/** \name Internal Utility API
 * \{ */

LOOM_INLINE void ghash_entry_copy ( GHash *gh_dist , Entry *dst , const GHash *gh_src , Entry *src , GHashKeyCopyFP keycopyfp , GHashValCopyFP valcopyfp ) {
	dst->key = ( keycopyfp ) ? keycopyfp ( src->key ) : src->key;

	if ( ( gh_dist->flag & GHASH_FLAG_IS_GSET ) == 0 ) {
		if ( ( gh_src->flag & GHASH_FLAG_IS_GSET ) == 0 ) {
			( ( GHashEntry * ) dst )->val = ( valcopyfp ) ? valcopyfp ( ( ( GHashEntry * ) src )->val ) : ( ( GHashEntry * ) src )->val;
		} else {
			( ( GHashEntry * ) dst )->val = NULL;
		}
	}
}

LOOM_INLINE unsigned int ghash_keyhash ( const GHash *gh , const void *key ) {
	return gh->hashfp ( key );
}

LOOM_INLINE unsigned int ghash_entryhash ( const GHash *gh , const Entry *e ) {
	return gh->hashfp ( e->key );
}

LOOM_INLINE unsigned int ghash_bucket_index ( const GHash *gh , const unsigned int hash ) {
#ifdef GHASH_USE_MODULO_BUCKETS
	return hash % gh->nbuckets;
#else
	return hash % gh->bucket_mask;
#endif
}

// Find the index of next used bucket, starting from \a curr_bucket (\a gh is assumed to be non-empty)
LOOM_INLINE unsigned int ghash_find_next_bucket_index ( const GHash *gh , unsigned int curr_bucket ) {
	if ( curr_bucket >= gh->nbuckets ) {
		curr_bucket = 0;
	}
	if ( gh->buckets [ curr_bucket ] ) {
		return curr_bucket;
	}
	for ( ; curr_bucket < gh->nbuckets; curr_bucket++ ) {
		if ( gh->buckets [ curr_bucket ] ) {
			return curr_bucket;
		}
	}
	for ( curr_bucket = 0; curr_bucket < gh->nbuckets; curr_bucket++ ) {
		if ( gh->buckets [ curr_bucket ] ) {
			return curr_bucket;
		}
	}
	LOOM_assert_unreachable ( );
	return 0;
}

// Expand buckets to the next size up or down
static void ghash_buckets_resize ( GHash *gh , const unsigned int nbuckets ) {
	Entry **buckets_old = gh->buckets;
	Entry **buckets_new;

	const unsigned int nbuckets_old = gh->nbuckets;

	unsigned int i;

	LOOM_assert ( ( gh->nbuckets != nbuckets ) || !gh->buckets );

	gh->nbuckets = nbuckets;
#ifdef GHASH_USE_MODULO_BUCKETS
#else
	gh->bucket_mask = nbuckets - 1;
#endif

	buckets_new = ( Entry ** ) MEM_callocN ( sizeof ( *gh->buckets ) * gh->nbuckets , __func__ );

	if ( buckets_old ) {
		if ( nbuckets > nbuckets_old ) {
			for ( i = 0; i < nbuckets_old; i++ ) {
				for ( Entry *e = buckets_old [ i ] , *e_next; e; e = e_next ) {
					const unsigned int hash = ghash_entryhash ( gh , e );
					const unsigned int bucket_index = ghash_bucket_index ( gh , hash );
					e_next = e->next;
					e->next = buckets_new [ bucket_index ];
					buckets_new [ bucket_index ] = e;
				}
			}
		} else {
			for ( i = 0; i < nbuckets_old; i++ ) {
			#ifdef GHASH_USE_MODULO_BUCKETS
				for ( Entry *e = buckets_old [ i ] , *e_next; e; e = e_next ) {
					const unsigned int hash = ghash_entryhash ( gh , e );
					const unsigned int bucket_index = ghash_bucket_index ( gh , hash );
					e_next = e->next;
					e->next = buckets_new [ bucket_index ];
					buckets_new [ bucket_index ] = e;
				}
			#else
				/* No need to recompute hashes in this case, since our mask is just
				 * smaller, all items in old bucket 'i' will go in same new bucket
				 * (i & new_mask)! */
				const unsigned int bucket_index = ghash_bucket_index ( gh , i );
				LOOM_assert ( !buckets_old [ i ] || ( bucket_index == ghash_bucket_index ( gh , ghash_entryhash ( gh , buckets_old [ i ] ) ) ) );
				Entry *e;
				for ( e = buckets_old [ i ]; e && e->next; e = e->next ) {
					/* pass */
				}
				if ( e ) {
					e->next = buckets_new [ bucket_index ];
					buckets_new [ bucket_index ] = buckets_old [ i ];
				}
			#endif
			}
		}
	}

	gh->buckets = buckets_new;
	if ( buckets_old ) {
		MEM_freeN ( buckets_old );
	}
}

// Check if the number of item in the GHash is large enough to require more buckets
static void ghash_buckets_expand ( GHash *gh , const unsigned int nentries , const bool user_defined ) {
	unsigned int new_nbuckets;

	if ( gh->buckets && ( nentries < gh->limit_grow ) ) {
		return;
	}

	new_nbuckets = gh->nbuckets;

#ifdef GHASH_USE_MODULO_BUCKETS
	while ( ( nentries > gh->limit_grow ) && ( gh->cursize < GHASH_MAX_SIZE - 1 ) ) {
		new_nbuckets = hashsizes [ ++gh->cursize ];
		gh->limit_grow = GHASH_LIMIT_GROW ( new_nbuckets );
	}
#else
	while ( ( nentries > gh->limit_grow ) && ( gh->bucket_bit < GHASH_BUCKET_BIT_MAX ) ) {
		new_nbuckets = 1u << ++gh->bucket_bit;
		gh->limit_grow = GHASH_LIMIT_GROW ( new_nbuckets );
	}
#endif

	if ( user_defined ) {
	#ifdef GHASH_USE_MODULO_BUCKETS
		gh->size_min = gh->cursize;
	#else
		gh->bucket_bit_min = gh->bucket_bit;
	#endif
	}

	if ( ( new_nbuckets == gh->nbuckets ) && gh->buckets ) {
		return;	 // nothing to do.
	}

	gh->limit_grow = GHASH_LIMIT_GROW ( new_nbuckets );
	gh->limit_shrink = GHASH_LIMIT_SHRINK ( new_nbuckets );
	ghash_buckets_resize ( gh , new_nbuckets );
}

static void ghash_buckets_contract ( GHash *gh , const unsigned int nentries , const bool user_defined , const bool force_shrink ) {
	unsigned int new_nbuckets;

	if ( !( force_shrink || ( gh->flag & GHASH_FLAG_ALLOW_SHRINK ) ) ) {
		return;
	}

	if ( gh->buckets && ( nentries > gh->limit_shrink ) ) {
		return;
	}

	new_nbuckets = gh->nbuckets;

#ifdef GHASH_USE_MODULO_BUCKETS
	while ( ( nentries > gh->limit_shrink ) && ( gh->cursize > gh->size_min ) ) {
		new_nbuckets = hashsizes [ --gh->cursize ];
		gh->limit_grow = GHASH_LIMIT_SHRINK ( new_nbuckets );
	}
#else
	while ( ( nentries > gh->limit_shrink ) && ( gh->bucket_bit > gh->bucket_bit_min ) ) {
		new_nbuckets = 1u << --gh->bucket_bit;
		gh->limit_grow = GHASH_LIMIT_SHRINK ( new_nbuckets );
	}
#endif

	if ( user_defined ) {
	#ifdef GHASH_USE_MODULO_BUCKETS
		gh->size_min = gh->cursize;
	#else
		gh->bucket_bit_min = gh->bucket_bit;
	#endif
	}

	if ( ( new_nbuckets == gh->nbuckets ) && gh->buckets ) {
		return;	 // nothing to do.
	}

	gh->limit_grow = GHASH_LIMIT_GROW ( new_nbuckets );
	gh->limit_shrink = GHASH_LIMIT_SHRINK ( new_nbuckets );
	ghash_buckets_resize ( gh , new_nbuckets );
}

// Clear and reset \a gh buckets, reserve again buckets for given number of entries.
LOOM_INLINE void ghash_buckets_reset ( GHash *gh , const unsigned int nentries ) {
	MEM_SAFE_FREE ( gh->buckets );

#ifdef GHASH_USE_MODULO_BUCKETS
	gh->cursize = 0;
	gh->size_min = 0;
	gh->nbuckets = hashsizes [ gh->cursize ];
#else
	gh->bucket_bit = GHASH_BUCKET_BIT_MIN;
	gh->bucket_bit_min = GHASH_BUCKET_BIT_MIN;
	gh->nbuckets = 1u << gh->bucket_bit;
	gh->bucket_mask = gh->nbuckets - 1;
#endif

	gh->limit_grow = GHASH_LIMIT_GROW ( gh->nbuckets );
	gh->limit_shrink = GHASH_LIMIT_SHRINK ( gh->nbuckets );

	gh->nentries = 0;

	ghash_buckets_expand ( gh , nentries , ( nentries != 0 ) );
}

LOOM_INLINE Entry *ghash_lookup_entry_ex ( const GHash *gh , const void *key , const unsigned int bucket_index ) {
	Entry *e;

	for ( e = gh->buckets [ bucket_index ]; e; e = e->next ) {
		if ( gh->cmpfp ( key , e->key ) == false ) {
			return e;
		}
	}

	return NULL;
}

LOOM_INLINE Entry *ghash_lookup_entry_prev_ex ( const GHash *gh , const void *key , Entry **r_e_prev , const unsigned int bucket_index ) {
	Entry *e_prev , *e;

	for ( e_prev = NULL , e = gh->buckets [ bucket_index ]; e; e_prev = e , e = e->next ) {
		if ( gh->cmpfp ( key , e->key ) == false ) {
			*r_e_prev = e_prev;
			return e;
		}
	}

	*r_e_prev = NULL;
	return NULL;
}

LOOM_INLINE Entry *ghash_lookup_entry ( const GHash *gh , const void *key ) {
	const unsigned int hash = ghash_keyhash ( gh , key );
	const unsigned int bucket_index = ghash_bucket_index ( gh , hash );
	return ghash_lookup_entry_ex ( gh , key , bucket_index );
}

static GHash *ghash_new ( GHashHashFP hashfp , GHashCmpFP cmpfp , const char *info , const unsigned int nentries_reserve , const unsigned int flag ) {
	GHash *gh = MEM_mallocN ( sizeof ( *gh ) , info );

	gh->hashfp = hashfp;
	gh->cmpfp = cmpfp;

	gh->buckets = NULL;
	gh->flag = flag;

	ghash_buckets_reset ( gh , nentries_reserve );
	gh->entrypool = GLU_mempool_create ( GHASH_ENTRY_SIZE ( flag & GHASH_FLAG_IS_GSET ) , 64 , 64 , LOOM_MEMPOOL_NOP );

	return gh;
}

LOOM_INLINE void ghash_insert_ex ( GHash *gh , void *key , void *val , const unsigned int bucket_index ) {
	GHashEntry *e = GLU_mempool_alloc ( gh->entrypool );

	LOOM_assert ( ( gh->flag & GHASH_FLAG_ALLOW_DUPS ) || ( GLU_ghash_haskey ( gh , key ) == 0 ) );
	LOOM_assert ( !( gh->flag & GHASH_FLAG_IS_GSET ) );

	e->e.next = gh->buckets [ bucket_index ];
	e->e.key = key;
	e->val = val;
	gh->buckets [ bucket_index ] = ( Entry * ) e;

	ghash_buckets_expand ( gh , ++gh->nentries , false );
}

LOOM_INLINE void ghash_insert_ex_keyonly_entry ( GHash *gh , void *key , const unsigned int bucket_index , Entry *e ) {
	LOOM_assert ( ( gh->flag & GHASH_FLAG_ALLOW_DUPS ) || ( GLU_ghash_haskey ( gh , key ) == 0 ) );

	e->next = gh->buckets [ bucket_index ];
	e->key = key;
	gh->buckets [ bucket_index ] = e;

	ghash_buckets_expand ( gh , ++gh->nentries , false );
}

LOOM_INLINE void ghash_insert_ex_keyonly ( GHash *gh , void *key , const unsigned int bucket_index ) {
	Entry *e = GLU_mempool_alloc ( gh->entrypool );

	LOOM_assert ( ( gh->flag & GHASH_FLAG_ALLOW_DUPS ) || ( GLU_ghash_haskey ( gh , key ) == 0 ) );
	LOOM_assert ( !( gh->flag & GHASH_FLAG_IS_GSET ) );

	e->next = gh->buckets [ bucket_index ];
	e->key = key;
	gh->buckets [ bucket_index ] = e;

	ghash_buckets_expand ( gh , ++gh->nentries , false );
}

LOOM_INLINE void ghash_insert ( GHash *gh , void *key , void *val ) {
	const unsigned int hash = ghash_keyhash ( gh , key );
	const unsigned int bucket_index = ghash_bucket_index ( gh , hash );

	ghash_insert_ex ( gh , key , val , bucket_index );
}

LOOM_INLINE bool ghash_insert_safe ( GHash *gh , void *key , void *val , const bool override , GHashKeyFreeFP keyfreefp , GHashValFreeFP valfreefp ) {
	const unsigned int hash = ghash_keyhash ( gh , key );
	const unsigned int bucket_index = ghash_bucket_index ( gh , hash );

	GHashEntry *e = ( GHashEntry * ) ghash_lookup_entry_ex ( gh , key , bucket_index );

	LOOM_assert ( !( gh->flag & GHASH_FLAG_IS_GSET ) );

	if ( e ) {
		if ( override ) {
			if ( keyfreefp ) {
				keyfreefp ( e->e.key );
			}
			if ( valfreefp ) {
				valfreefp ( e->val );
			}
		}
		e->e.key = key;
		e->val = val;
		return false;
	}

	ghash_insert_ex ( gh , key , val , bucket_index );
	return true;
}

LOOM_INLINE bool ghash_insert_safe_keyonly ( GHash *gh , void *key , const bool override , GHashKeyFreeFP keyfreefp ) {
	const unsigned int hash = ghash_keyhash ( gh , key );
	const unsigned int bucket_index = ghash_bucket_index ( gh , hash );

	Entry *e = ghash_lookup_entry_ex ( gh , key , bucket_index );

	LOOM_assert ( ( gh->flag & GHASH_FLAG_IS_GSET ) != 0 );

	if ( e ) {
		if ( override ) {
			if ( keyfreefp ) {
				keyfreefp ( e->key );
			}
			e->key = key;
		}
		return false;
	}

	ghash_insert_ex_keyonly ( gh , key , bucket_index );
	return true;
}

static Entry *ghash_remove_ex ( GHash *gh , const void *key , GHashKeyFreeFP keyfreefp , GHashValFreeFP valfreefp , const unsigned int bucket_index ) {
	Entry *e_prev;
	Entry *e = ghash_lookup_entry_prev_ex ( gh , key , &e_prev , bucket_index );

	LOOM_assert ( !valfreefp || !( gh->flag & GHASH_FLAG_IS_GSET ) );

	if ( e ) {
		if ( keyfreefp ) {
			keyfreefp ( e->key );
		}
		if ( valfreefp ) {
			valfreefp ( ( ( GHashEntry * ) e )->val );
		}

		if ( e_prev ) {
			e_prev->next = e->next;
		} else {
			gh->buckets [ bucket_index ] = e->next;
		}

		ghash_buckets_contract ( gh , --gh->nentries , false , false );
	}

	return e;
}

static Entry *ghash_pop ( GHash *gh , GHashIterState *state ) {
	unsigned int curr_bucket = state->CurrentBucket;
	if ( gh->nentries == 0 ) {
		return NULL;
	}

	/* NOTE: using first_bucket_index here allows us to avoid potential
	 * huge number of loops over buckets,
	 * in case we are popping from a large ghash with few items in it... */
	curr_bucket = ghash_find_next_bucket_index ( gh , curr_bucket );

	Entry *e = gh->buckets [ curr_bucket ];
	LOOM_assert ( e );

	ghash_remove_ex ( gh , e->key , NULL , NULL , curr_bucket );

	state->CurrentBucket = curr_bucket;
	return e;
}

static void ghash_free_cb ( GHash *gh , GHashKeyFreeFP keyfreefp , GHashValFreeFP valfreefp ) {
	unsigned int i;

	LOOM_assert ( keyfreefp || valfreefp );
	LOOM_assert ( !valfreefp || !( gh->flag & GHASH_FLAG_IS_GSET ) );

	for ( i = 0; i < gh->nbuckets; i++ ) {
		Entry *e;

		for ( e = gh->buckets [ i ]; e; e = e->next ) {
			if ( keyfreefp ) {
				keyfreefp ( e->key );
			}
			if ( valfreefp ) {
				valfreefp ( ( ( GHashEntry * ) e )->val );
			}
		}
	}
}

static GHash *ghash_copy ( const GHash *gh , GHashKeyCopyFP keycopyfp , GHashValCopyFP valcopyfp ) {
	GHash *gh_new;
	unsigned int i;
	/* This allows us to be sure to get the same number of buckets in gh_new as in ghash. */
	const unsigned int reserve_nentries_new = MAX2 ( GHASH_LIMIT_GROW ( gh->nbuckets ) - 1 , gh->nentries );

	LOOM_assert ( !valcopyfp || !( gh->flag & GHASH_FLAG_IS_GSET ) );

	gh_new = ghash_new ( gh->hashfp , gh->cmpfp , __func__ , 0 , gh->flag );
	ghash_buckets_expand ( gh_new , reserve_nentries_new , false );

	LOOM_assert ( gh_new->nbuckets == gh->nbuckets );

	for ( i = 0; i < gh->nbuckets; i++ ) {
		Entry *e;

		for ( e = gh->buckets [ i ]; e; e = e->next ) {
			Entry *e_new = GLU_mempool_alloc ( gh_new->entrypool );
			ghash_entry_copy ( gh_new , e_new , gh , e , keycopyfp , valcopyfp );

			/* Warning!
			 * This means entries in buckets in new copy will be in reversed order!
			 * This shall not be an issue though, since order should never be assumed
			 * in ghash. */

			 /* NOTE: We can use 'i' here, since we are sure that
			  * 'gh' and 'gh_new' have the same number of buckets! */
			e_new->next = gh_new->buckets [ i ];
			gh_new->buckets [ i ] = e_new;
		}
	}
	gh_new->nentries = gh->nentries;

	return gh_new;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name GHash Public API
 * \{ */

GHash *GLU_ghash_new_ex ( GHashHashFP hashfp , GHashCmpFP cmpfp , const char *info , const unsigned int nentries_reserve ) {
	return ghash_new ( hashfp , cmpfp , info , nentries_reserve , 0 );
}

GHash *GLU_ghash_new ( GHashHashFP hashfp , GHashCmpFP cmpfp , const char *info ) {
	return GLU_ghash_new_ex ( hashfp , cmpfp , info , 0 );
}

GHash *GLU_ghash_copy ( const GHash *gh , GHashKeyCopyFP keycopyfp , GHashValCopyFP valcopyfp ) {
	return ghash_copy ( gh , keycopyfp , valcopyfp );
}

void GLU_ghash_reserve ( GHash *gh , const unsigned int nentries_reserve ) {
	ghash_buckets_expand ( gh , nentries_reserve , true );
	ghash_buckets_contract ( gh , nentries_reserve , true , false );
}

unsigned int GLU_ghash_len ( const GHash *gh ) {
	return gh->nentries;
}

void GLU_ghash_insert ( GHash *gh , void *key , void *val ) {
	ghash_insert ( gh , key , val );
}

bool GLU_ghash_reinsert ( GHash *gh , void *key , void *val , GHashKeyFreeFP keyfreefp , GHashValFreeFP valfreefp ) {
	return ghash_insert_safe ( gh , key , val , true , keyfreefp , valfreefp );
}

void *GLU_ghash_replace_key ( GHash *gh , void *key ) {
	const unsigned int hash = ghash_keyhash ( gh , key );
	const unsigned int bucket_index = ghash_bucket_index ( gh , hash );
	GHashEntry *e = ( GHashEntry * ) ghash_lookup_entry_ex ( gh , key , bucket_index );
	if ( e != NULL ) {
		void *key_prev = e->e.key;
		e->e.key = key;
		return key_prev;
	}
	return NULL;
}

void *GLU_ghash_lookup ( const GHash *gh , const void *key ) {
	GHashEntry *e = ( GHashEntry * ) ghash_lookup_entry ( gh , key );
	LOOM_assert ( !( gh->flag & GHASH_FLAG_IS_GSET ) );
	return e ? e->val : NULL;
}

void *GLU_ghash_lookup_default ( const GHash *gh , const void *key , void *val_default ) {
	GHashEntry *e = ( GHashEntry * ) ghash_lookup_entry ( gh , key );
	LOOM_assert ( !( gh->flag & GHASH_FLAG_IS_GSET ) );
	return e ? e->val : val_default;
}

void **GLU_ghash_lookup_p ( GHash *gh , const void *key ) {
	GHashEntry *e = ( GHashEntry * ) ghash_lookup_entry ( gh , key );
	LOOM_assert ( !( gh->flag & GHASH_FLAG_IS_GSET ) );
	return e ? &e->val : NULL;
}

bool GLU_ghash_ensure_p ( GHash *gh , void *key , void ***r_val ) {
	const unsigned int hash = ghash_keyhash ( gh , key );
	const unsigned int bucket_index = ghash_bucket_index ( gh , hash );
	GHashEntry *e = ( GHashEntry * ) ghash_lookup_entry_ex ( gh , key , bucket_index );
	const bool haskey = ( e != NULL );

	if ( !haskey ) {
		e = GLU_mempool_alloc ( gh->entrypool );
		ghash_insert_ex_keyonly_entry ( gh , key , bucket_index , ( Entry * ) e );
	}

	*r_val = &e->val;
	return haskey;
}

bool GLU_ghash_ensure_p_ex ( GHash *gh , const void *key , void ***r_key , void ***r_val ) {
	const unsigned int hash = ghash_keyhash ( gh , key );
	const unsigned int bucket_index = ghash_bucket_index ( gh , hash );
	GHashEntry *e = ( GHashEntry * ) ghash_lookup_entry_ex ( gh , key , bucket_index );
	const bool haskey = ( e != NULL );

	if ( !haskey ) {
		/* Pass 'key' in case we resize. */
		e = GLU_mempool_alloc ( gh->entrypool );
		ghash_insert_ex_keyonly_entry ( gh , ( void * ) key , bucket_index , ( Entry * ) e );
		e->e.key = NULL; /* caller must re-assign */
	}

	*r_key = &e->e.key;
	*r_val = &e->val;
	return haskey;
}

bool GLU_ghash_remove ( GHash *gh , const void *key , GHashKeyFreeFP keyfreefp , GHashValFreeFP valfreefp ) {
	const unsigned int hash = ghash_keyhash ( gh , key );
	const unsigned int bucket_index = ghash_bucket_index ( gh , hash );
	Entry *e = ghash_remove_ex ( gh , key , keyfreefp , valfreefp , bucket_index );
	if ( e ) {
		GLU_mempool_free ( gh->entrypool , e );
		return true;
	}
	return false;
}

void *GLU_ghash_popkey ( GHash *gh , const void *key , GHashKeyFreeFP keyfreefp ) {
	/* Same as above but return the value,
	 * no free value argument since it will be returned. */

	const unsigned int hash = ghash_keyhash ( gh , key );
	const unsigned int bucket_index = ghash_bucket_index ( gh , hash );
	GHashEntry *e = ( GHashEntry * ) ghash_remove_ex ( gh , key , keyfreefp , NULL , bucket_index );
	LOOM_assert ( !( gh->flag & GHASH_FLAG_IS_GSET ) );
	if ( e ) {
		void *val = e->val;
		GLU_mempool_free ( gh->entrypool , e );
		return val;
	}
	return NULL;
}

bool GLU_ghash_haskey ( const GHash *gh , const void *key ) {
	return ( ghash_lookup_entry ( gh , key ) != NULL );
}

bool GLU_ghash_pop ( GHash *gh , GHashIterState *state , void **r_key , void **r_val ) {
	GHashEntry *e = ( GHashEntry * ) ghash_pop ( gh , state );

	LOOM_assert ( !( gh->flag & GHASH_FLAG_IS_GSET ) );

	if ( e ) {
		*r_key = e->e.key;
		*r_val = e->val;

		GLU_mempool_free ( gh->entrypool , e );
		return true;
	}

	*r_key = *r_val = NULL;
	return false;
}

void GLU_ghash_clear_ex ( GHash *gh , GHashKeyFreeFP keyfreefp , GHashValFreeFP valfreefp , const unsigned int nentries_reserve ) {
	if ( keyfreefp || valfreefp ) {
		ghash_free_cb ( gh , keyfreefp , valfreefp );
	}

	ghash_buckets_reset ( gh , nentries_reserve );
	GLU_mempool_clear_ex ( gh->entrypool , nentries_reserve ? ( int ) nentries_reserve : -1 );
}

void GLU_ghash_clear ( GHash *gh , GHashKeyFreeFP keyfreefp , GHashValFreeFP valfreefp ) {
	GLU_ghash_clear_ex ( gh , keyfreefp , valfreefp , 0 );
}

void GLU_ghash_free ( GHash *gh , GHashKeyFreeFP keyfreefp , GHashValFreeFP valfreefp ) {
	LOOM_assert ( ( int ) gh->nentries == GLU_mempool_len ( gh->entrypool ) );
	if ( keyfreefp || valfreefp ) {
		ghash_free_cb ( gh , keyfreefp , valfreefp );
	}

	MEM_freeN ( gh->buckets );
	GLU_mempool_discard ( gh->entrypool );
	MEM_freeN ( gh );
}

void GLU_ghash_flag_set ( GHash *gh , unsigned int flag ) {
	gh->flag |= flag;
}

void GLU_ghash_flag_clear ( GHash *gh , unsigned int flag ) {
	gh->flag &= ~flag;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name GHash Iterator API
 * \{ */

GHashIterator *GLU_ghash_iterator_new ( GHash *gh ) {
	GHashIterator *ghi = MEM_mallocN ( sizeof ( *ghi ) , "ghash iterator" );
	GLU_ghash_iterator_init ( ghi , gh );
	return ghi;
}

void GLU_ghash_iterator_init ( GHashIterator *ghi , GHash *gh ) {
	ghi->Container = gh;
	ghi->CurrentEntry = NULL;
	ghi->CurrentBucket = UINT_MAX; /* wraps to zero */
	if ( gh->nentries ) {
		do {
			ghi->CurrentBucket++;
			if ( ghi->CurrentBucket == ghi->Container->nbuckets ) {
				break;
			}
			ghi->CurrentEntry = ghi->Container->buckets [ ghi->CurrentBucket ];
		} while ( !ghi->CurrentEntry );
	}
}

void GLU_ghash_iterator_step ( GHashIterator *ghi ) {
	if ( ghi->CurrentEntry ) {
		ghi->CurrentEntry = ghi->CurrentEntry->next;
		while ( !ghi->CurrentEntry ) {
			ghi->CurrentBucket++;
			if ( ghi->CurrentBucket == ghi->Container->nbuckets ) {
				break;
			}
			ghi->CurrentEntry = ghi->Container->buckets [ ghi->CurrentBucket ];
		}
	}
}

void GLU_ghash_iterator_free ( GHashIterator *ghi ) {
	MEM_freeN ( ghi );
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name GSet Public API
 * \{ */

GSet *GLU_gset_new_ex ( GSetHashFP hashfp , GSetCmpFP cmpfp , const char *info , const unsigned int nentries_reserve ) {
	return ( GSet * ) ghash_new ( hashfp , cmpfp , info , nentries_reserve , GHASH_FLAG_IS_GSET );
}

GSet *GLU_gset_new ( GSetHashFP hashfp , GSetCmpFP cmpfp , const char *info ) {
	return GLU_gset_new_ex ( hashfp , cmpfp , info , 0 );
}

GSet *GLU_gset_copy ( const GSet *gs , GHashKeyCopyFP keycopyfp ) {
	return ( GSet * ) ghash_copy ( ( const GHash * ) gs , keycopyfp , NULL );
}

unsigned int GLU_gset_len ( const GSet *gs ) {
	return ( ( GHash * ) gs )->nentries;
}

void GLU_gset_insert ( GSet *gs , void *key ) {
	const unsigned int hash = ghash_keyhash ( ( GHash * ) gs , key );
	const unsigned int bucket_index = ghash_bucket_index ( ( GHash * ) gs , hash );
	ghash_insert_ex_keyonly ( ( GHash * ) gs , key , bucket_index );
}

bool GLU_gset_add ( GSet *gs , void *key ) {
	return ghash_insert_safe_keyonly ( ( GHash * ) gs , key , false , NULL );
}

bool GLU_gset_ensure_p_ex ( GSet *gs , const void *key , void ***r_key ) {
	const unsigned int hash = ghash_keyhash ( ( GHash * ) gs , key );
	const unsigned int bucket_index = ghash_bucket_index ( ( GHash * ) gs , hash );
	GSetEntry *e = ( GSetEntry * ) ghash_lookup_entry_ex ( ( const GHash * ) gs , key , bucket_index );
	const bool haskey = ( e != NULL );

	if ( !haskey ) {
		/* Pass 'key' in case we resize */
		e = GLU_mempool_alloc ( ( ( GHash * ) gs )->entrypool );
		ghash_insert_ex_keyonly_entry ( ( GHash * ) gs , ( void * ) key , bucket_index , ( Entry * ) e );
		e->key = NULL; /* caller must re-assign */
	}

	*r_key = &e->key;
	return haskey;
}

bool GLU_gset_reinsert ( GSet *gs , void *key , GSetKeyFreeFP keyfreefp ) {
	return ghash_insert_safe_keyonly ( ( GHash * ) gs , key , true , keyfreefp );
}

void *GLU_gset_replace_key ( GSet *gs , void *key ) {
	return GLU_ghash_replace_key ( ( GHash * ) gs , key );
}

bool GLU_gset_remove ( GSet *gs , const void *key , GSetKeyFreeFP keyfreefp ) {
	return GLU_ghash_remove ( ( GHash * ) gs , key , keyfreefp , NULL );
}

bool GLU_gset_haskey ( const GSet *gs , const void *key ) {
	return ( ghash_lookup_entry ( ( const GHash * ) gs , key ) != NULL );
}

bool GLU_gset_pop ( GSet *gs , GSetIterState *state , void **r_key ) {
	GSetEntry *e = ( GSetEntry * ) ghash_pop ( ( GHash * ) gs , ( GHashIterState * ) state );

	if ( e ) {
		*r_key = e->key;

		GLU_mempool_free ( ( ( GHash * ) gs )->entrypool , e );
		return true;
	}

	*r_key = NULL;
	return false;
}

void GLU_gset_clear_ex ( GSet *gs , GSetKeyFreeFP keyfreefp , const unsigned int nentries_reserve ) {
	GLU_ghash_clear_ex ( ( GHash * ) gs , keyfreefp , NULL , nentries_reserve );
}

void GLU_gset_clear ( GSet *gs , GSetKeyFreeFP keyfreefp ) {
	GLU_ghash_clear ( ( GHash * ) gs , keyfreefp , NULL );
}

void GLU_gset_free ( GSet *gs , GSetKeyFreeFP keyfreefp ) {
	GLU_ghash_free ( ( GHash * ) gs , keyfreefp , NULL );
}

void GLU_gset_flag_set ( GSet *gs , unsigned int flag ) {
	( ( GHash * ) gs )->flag |= flag;
}

void GLU_gset_flag_clear ( GSet *gs , unsigned int flag ) {
	( ( GHash * ) gs )->flag &= ~flag;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name GSet Combined Key/Value Usage
 *
 * \note Not typical `set` use, only use when the pointer identity matters.
 * This can be useful when the key references data stored outside the GSet.
 * \{ */

void *GLU_gset_lookup ( const GSet *gs , const void *key ) {
	Entry *e = ghash_lookup_entry ( ( const GHash * ) gs , key );
	return e ? e->key : NULL;
}

void *GLU_gset_pop_key ( GSet *gs , const void *key ) {
	const unsigned int hash = ghash_keyhash ( ( GHash * ) gs , key );
	const unsigned int bucket_index = ghash_bucket_index ( ( GHash * ) gs , hash );
	Entry *e = ghash_remove_ex ( ( GHash * ) gs , key , NULL , NULL , bucket_index );
	if ( e ) {
		void *key_ret = e->key;
		GLU_mempool_free ( ( ( GHash * ) gs )->entrypool , e );
		return key_ret;
	}
	return NULL;
}

/** \} */
