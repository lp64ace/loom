#pragma once

#include "loomlib_assert.h"
#include "loomlib_compiler.h"
#include "loomlib_utildefines.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------- */
/** \name GHash Types
 * \{ */

typedef unsigned int (*GHashHashFP)(const void *key);
typedef bool (*GHashCmpFP)(const void *a,
						   const void *b);	// returns false when equal
typedef void (*GHashKeyFreeFP)(void *key);
typedef void (*GHashValFreeFP)(void *val);
typedef void *(*GHashKeyCopyFP)(const void *key);
typedef void *(*GHashValCopyFP)(const void *val);

typedef struct GHash GHash;

typedef struct GHashIterator {
	GHash *Container;
	struct Entry *CurrentEntry;
	unsigned int CurrentBucket;
} GHashIterator;

typedef struct GHashIterState {
	unsigned int CurrentBucket;
} GHashIterState;

enum {
	/**
	 * This flag only has meaning in debug mode, it is only checked in
	 * debug mode.
	 */
	GHASH_FLAG_ALLOW_DUPS = (1 << 0),

	/**
	 * Allow the buckets' size to shrink.
	 */
	GHASH_FLAG_ALLOW_SHRINK = (1 << 1),

	/**
	 * Internal usage only, whether the GHash is actually a GSet meaning that it
	 * contains no value storage.
	 */
	GHASH_FLAG_IS_GSET = (1 << 16),
};

/** \} */

/* -------------------------------------------------------------------- */
/** \name GHash API
 * \{ */

/**
 * Creates a new, empty GHash with the specified creation params.
 * \param hash_fp: Hash callback.
 * \param cmp_fp: Comparison callback.
 * \param info: Identifier string for the GHash.
 * \param nentries_reserve: Optionally reserve the number of members that the
 * hash will hold. Use this to avoid resizing buckets if the size is known or
 * can be closely approximated. \return  An empty GHash.
 */
GHash *GLU_ghash_new_ex(GHashHashFP hash_fp,
						GHashCmpFP cmp_fp,
						const char *info,
						unsigned int nentries_reserve);

/**
 * Creates a new, empty GHash with the no entries reserved
 * same as calling #GLU_ghash_new_ex(...,0).
 * \param hash_fp: Hash callback.
 * \param cmp_fp: Comparison callback.
 * \param info: Identifier string for the GHash.
 */
GHash *GLU_ghash_new(GHashHashFP hash_fp, GHashCmpFP cmp_fp, const char *info);

/**
 * Copy given GHash. Keys and values are also copied if relevant callback is
 * provided, else pointers remain the same.
 */
GHash *GLU_ghash_copy(const GHash *gh,
					  GHashKeyCopyFP keycopy_fp,
					  GHashValCopyFP valcopy_fp);

/**
 * Frees the GHash and its members.
 * \param gh: The GHash to free.
 * \param keyfreefp: Optional callback to free the key.
 * \param valfreefp: Optional callback to free the value.
 */
void GLU_ghash_free(GHash *gh,
					GHashKeyFreeFP keyfreefp,
					GHashValFreeFP valfreefp);

/**
 * Reserve given amount of entries (resize \a gh accordingly if needed).
 * \param gh: The GHash to reserve entries into.
 * \param nentrie_reserve: reserve the number of members that the hash will
 * hold. Use this to avoid resizing buckets if the size is known or can be
 * closely approximated.
 */
void GLU_ghash_reserver(GHash *gh, unsigned int nentries_reserve);

/**
 * Insert a key/value pair into the \a gh.
 * \note Duplicates are not checked,
 * the caller is expected to ensure elements are unique unless
 * GHASH_FLAG_ALLOW_DUPES flag is set.
 */
void GLU_ghash_insert(GHash *gh, void *key, void *val);

/**
 * Inserts a new value to a key that may already be in ghash.
 * Avoids #GLU_ghash_remove, #GLU_ghash_insert calls (double lookups)
 * \returns true if a new key has been added.
 */
bool GLU_ghash_reinsert(GHash *gh,
						void *key,
						void *val,
						GHashKeyFreeFP keyfree_fp,
						GHashValFreeFP valfree_fp);

/**
 * Replaces the key of an item in the \a gh.
 * Use when a key is re-allocated or its memory location is changed.
 * \returns The previous key or NULL if not found, the caller may free if it's
 * needed.
 */
void *GLU_ghash_replace_key(GHash *gh, void *key);

/**
 * Lookup the value of \a key in \a gh.
 * \param key: The key to lookup.
 * \returns the value for \a key or NULL.
 * \note When NULL is a valid value, use #GLU_ghash_lookup_p to differentiate a
 * missing key from a key with a NULL value. (Avoids calling #GLU_ghash_haskey
 * before #GLU_ghash_lookup)
 */
void *GLU_ghash_lookup(GHash *gh, void *key);

/** A version of #GLU_ghash_lookup which accepts a fallback argument. */
void *GLU_ghash_lookup_default(const GHash *gh,
							   const void *key,
							   void *val_default);

/**
 * Lookup a pointer to the value of \a key in \a gh.
 * \param key: The key to lookup.
 * \returns the pointer to value for \a key or NULL.
 * \note This has 2 main benefits over #GLU_ghash_lookup.
 * - A NULL return always means that \a key isn't in \a gh.
 * - The value can be modified in-place without further function calls (faster).
 */
void **GLU_ghash_lookup_p(GHash *gh, const void *key);

/**
 * Ensure \a key is exists in \a gh.
 * This handles the common situation where the caller needs ensure a key is
 * added to \a gh, constructing a new value in the case the key isn't found.
 * Otherwise use the existing value.
 *
 * Such situations typically incur multiple lookups, however this function
 * avoids them by ensuring the key is added,
 * returning a pointer to the value so it can be used or initialized by the
 * caller.
 *
 * \returns true when the value didn't need to be added.
 * (when false, the caller _must_ initialize the value).
 */
bool GLU_ghash_ensure_p(GHash *gh, void *key, void ***r_val);

/**
 * A version of #GLU_ghash_ensure_p that allows caller to re-assign the key.
 * Typically used when the key is to be duplicated.
 *
 * \warning Caller _must_ write to \a r_key when returning false.
 */
bool GLU_ghash_ensure_p_ex(GHash *gh,
						   const void *key,
						   void ***r_key,
						   void ***r_val);

/**
 * Remove \a key from \a gh, or return false if the key wasn't found.
 * \param key: The key to remove.
 * \param keyfreefp: Optional callback to free the key.
 * \param valfreefp: Optional callback to free the value.
 * \return true if \a key was removed from \a gh.
 */
bool GLU_ghash_remove(GHash *gh,
					  const void *key,
					  GHashKeyFreeFP keyfree_fp,
					  GHashValFreeFP valfree_fp);

/**
 * Reset \a gh clearing all entries.
 * \param keyfreefp: Optional callback to free the key.
 * \param valfreefp: Optional callback to free the value.
 * \param nentries_reserve: Optionally reserve the number of members that the
 * hash will hold.
 */
void GLU_ghash_clear_exs(GHash *gh,
						 GHashKeyFreeFP keyfree_fp,
						 GHashValFreeFP valfree_fp,
						 unsigned int nentries_reserve);

// Wraps #GLU_ghash_clear_ex with zero entries reserved.
void GLU_ghash_clear(GHash *gh,
					 GHashKeyFreeFP keyfree_fp,
					 GHashValFreeFP valfree_fp);

/**
 * Remove \a key from \a gh, returning the value or NULL if the key wasn't
 * found. \param key: The key to remove. \param keyfreefp: Optional callback to
 * free the key. \return the value of \a key int \a gh or NULL.
 */
void *GLU_ghash_popkey(GHash *gh, const void *key, GHashKeyFreeFP keyfree_fp);

/** \return true if the \a key is in \a gh. */
bool GLU_ghash_haskey(const GHash *gh, const void *key);

/** Remove a random entry from \a gh, returning true
 * if a key/value pair could be removed, false otherwise.
 * \param r_key: The removed key.
 * \param r_val: The removed value.
 * \param state: Used for efficient removal.
 * \return true if there was something to pop, false if ghash was already empty.
 */
bool GLU_ghash_pop(GHash *gh,
				   GHashIterState *state,
				   void **r_key,
				   void **r_val);

// \return Returns the size of the \a gh.
unsigned int GLU_ghash_len(const GHash *gh);

// Sets a GHash flag.
void GLU_ghash_flag_set(GHash *gh, unsigned int flag);

// Clear a GHash flag.
void GLU_ghash_flag_clear(GHash *gh, unsigned int flag);

/** \} */

/* -------------------------------------------------------------------- */
/** \name GHash Iterator
 * \{ */

/**
 * Create a new GHashIterator. The hash table must not be mutated
 * while the iterator is in use, and the iterator will step exactly
 * #GLU_ghash_len(gh) time before becoming done.
 *
 * \param gh: The GHah to iterate over.
 * \return Pointer to new iterator.
 */
GHashIterator *GLU_ghash_iterator_new(GHash *gh);

/**
 * Init an already allocated GHashIterator. The hash table must not be mutated
 * while the iterator is in use, and the iterator will step exactly
 * #GLU_ghash_len(gh) time before becoming done.
 *
 * \param ghi: The GHashIterator to initialize.
 * \param gh: The GHash to iterate over.
 */
void GLU_ghash_iterator_init(GHashIterator *ghi, GHash *gh);

/**
 * Free the iterator (GHashIterator).
 * \param ghi: The iterator to free.
 */
void GLU_ghash_iterator_free(GHashIterator *ghi);

/**
 * Steps the iterator to the next index.
 * \param ghi: The iterator we want to step, the iterator has to
 * be properly initiallized.
 */
void GLU_ghash_iterator_step(GHashIterator *ghi);

LOOM_INLINE void *GLU_ghash_iterator_get_key(GHashIterator *ghi);
LOOM_INLINE void *GLU_ghash_iterator_get_value(GHashIterator *ghi);
LOOM_INLINE void **GLU_ghash_iterator_get_value_p(GHashIterator *ghi);
LOOM_INLINE bool GLU_ghash_iterator_done(const GHashIterator *ghi);

struct _gh_Entry {
	void *next, *key, *val;
};

LOOM_INLINE void *GLU_ghash_iterator_get_key(GHashIterator *ghi)
{
	return ((struct _gh_Entry *)ghi->CurrentEntry)->key;
}
LOOM_INLINE void *GLU_ghash_iterator_get_value(GHashIterator *ghi)
{
	return ((struct _gh_Entry *)ghi->CurrentEntry)->val;
}
LOOM_INLINE void **GLU_ghash_iterator_get_value_p(GHashIterator *ghi)
{
	return &((struct _gh_Entry *)ghi->CurrentEntry)->val;
}
LOOM_INLINE bool GLU_ghash_iterator_done(const GHashIterator *ghi)
{
	return !ghi->CurrentEntry;
}

#define _gh_Entry void

#define GHASH_ITER(gh_iter_, ghash_) \
	for (GLU_ghash_iterator_init(&gh_iter_, ghash_); \
		 GLU_ghash_iterator_done(&gh_iter_) == false; \
		 GLU_ghash_iterator_step(&gh_iter_))

#define GHASH_ITER_INDEX(gh_iter_, ghash_, i_) \
	for (GLU_ghash_iterator_init(&gh_iter_, ghash_), i_ = 0; \
		 GLU_ghash_iterator_done(&gh_iter_) == false; \
		 GLU_ghash_iterator_step(&gh_iter_), i_++)

/** \} */

/* -------------------------------------------------------------------- */
/** \name GSet Types
 * \{ */

typedef struct GSet GSet;

typedef GHashHashFP GSetHashFP;
typedef GHashCmpFP GSetCmpFP;
typedef GHashKeyFreeFP GSetKeyFreeFP;
typedef GHashValFreeFP GSetValFreeFP;
typedef GHashKeyCopyFP GSetKeyCopyFP;

typedef GHashIterState GSetIterState;

/** \} */

/* -------------------------------------------------------------------- */
/** \name GSet Types
 * \{ */

/**
 * Creates a new, empty GSet with the specified creation params.
 * \param hash_fp: Hash callback.
 * \param cmp_fp: Comparison callback.
 * \param info: Identifier string for the GSet.
 * \param nentries_reserve: Optionally reserve the number of members that the
 * hash will hold. Use this to avoid resizing buckets if the size is known or
 * can be closely approximated. \return  An empty GSet.
 */
GSet *GLU_gset_new_ex(GSetHashFP hashfp,
					  GSetCmpFP cmpfp,
					  const char *info,
					  unsigned int nentries_reserve);

/**
 * Creates a new, empty GSet with the specified creation params.
 * \param hash_fp: Hash callback.
 * \param cmp_fp: Comparison callback.
 * \param info: Identifier string for the GSet.
 * \return  An empty GHash.
 */
GSet *GLU_gset_new(GSetHashFP hashfp, GSetCmpFP cmpfp, const char *info);

/**
 * Copy given GSet. Keys are also copied if callback is provided, else pointers
 * remain the same. \param gset: The GSet to copy. \return Returns the newly
 * allocated GSet with the same contents as the one provided.
 */
GSet *GLU_gset_copy(const GSet *gs, GSetKeyCopyFP keycopyfp);

// \return Returns the size of the \a gs.
unsigned int GLU_gset_len(const GSet *gs);

// Sets a GSet flag.
void GLU_gset_flag_set(GSet *gs, unsigned int flag);

// Clear a GSet flag.
void GLU_gset_flag_clear(GSet *gs, unsigned int flag);

/**
 * Frees the GSet and its' memebers.
 * \param gs: The GSet to free.
 * \param keyfreefp: Optional function that will be used to free each key.
 */
void GLU_gset_free(GSet *gs, GSetKeyFreeFP keyfreefp);

/**
 * Adds the key to the set (no checks for unique keys!).
 * Matching #GLU_ghash_insert.
 */
void GLU_gset_insert(GSet *gs, void *key);

/**
 * A version of #GLU_ghash_insert which checks first if the key is in the set.
 * \returns True if a new key has been added.
 *
 * \note GHash has no equivalent to this because typically the value would be
 * different.
 */
bool GLU_gset_add(GSet *gs, void *key);

/**
 * Sets counterpart to #GLU_ghash_ensure_p_ex.
 * simlar to #GLU_gset_add, except it returns the key pointer.
 *
 * \warning Caller _must_ write to \a r_key when returning false.
 */
bool GLU_gset_ensure_p_ex(GSet *gs, const void *key, void ***r_key);

/**
 * Adds the key to the set (duplicates are managed).
 * Matching #GLU_ghash_reinsert.
 *
 * \return Returns true if a new key has been added.
 */
bool GLU_gset_reinsert(GSet *gs, void *key, GSetKeyFreeFP keyfreefp);

/**
 * Repalces the key to the set if it's found.
 * Matching #GLU_ghash_replace_key.
 *
 * \return Returns the old key or NULL if not found.
 */
void *GLU_gset_replace_key(GSet *gs, void *key);

bool GLU_gset_haskey(const GSet *gs, const void *key);

/**
 * Remove a random entry from \a gs, returning true if a key could be removed,
 * false otherwise.
 *
 * \param r_key The removed key.
 * \param state Used for efficient removal.
 * \return Returns true if there was something to pop, false if gset is already
 * empty.
 */
bool GLU_gset_pop(GSet *gs, GSetIterState *state, void **r_key);

/**
 * Removes the specified key from \a gs, returning true if the key could be
 * removed, false otherwise.
 *
 * \param key The key we want to remove.
 * \param keyfreefp Optional function to free the key.
 * \return Returns true if the \a key was removed, false if the \a key was not
 * found in the gset.
 */
bool GLU_gset_remove(GSet *gs, const void *key, GSetKeyFreeFP keyfreefp);

/**
 * Reset \a gs clearing all entries.
 * \param keyfreefp: Optional callback to free the key.
 * \param nentries_reserve: Optionally reserve the number of members that the
 * hash will hold.
 */
void GLU_gset_clear_ex(GSet *gs,
					   GSetKeyFreeFP keyfreefp,
					   unsigned int nentries_reserve);

/**
 * Reset \a gh clearing all entries.
 * \param keyfreefp: Optional callback to free the key.
 *
 * \note same as calling #GLU_gset_clear_ex(gs, keyfreefp)
 */
void GLU_gset_clear(GSet *gs, GSetKeyFreeFP keyfreefp);

/**
 * Searches the gset to find the specified \a key.
 * \return Returns the pointer to the key if it's found.
 */
void *GLU_gset_lookup(const GSet *gs, const void *key);

/**
 * Searches the gset to find the specified \a key and removes it.
 * \return Returns the pointer to the key if it's found, removing it from the
 * gset. \note Called must handle freeing.
 */
void *GLU_gset_pop_key(GSet *gs, const void *key);

/** \} */

/* -------------------------------------------------------------------- */
/** \name GSet Iterator
 * \{ */

typedef struct GSetIterator {
	GHashIterator _ghi;
} GSetIterator;

LOOM_INLINE GSetIterator *GLU_gset_iterator_new(GSet *gs)
{
	return (GSetIterator *)GLU_ghash_iterator_new((GHash *)gs);
}
LOOM_INLINE void GLU_gset_iterator_init(GSetIterator *gsi, GSet *gs)
{
	GLU_ghash_iterator_init((GHashIterator *)gsi, (GHash *)gs);
}
LOOM_INLINE void GLU_gset_iterator_free(GSetIterator *gsi)
{
	GLU_ghash_iterator_free((GHashIterator *)gsi);
}
LOOM_INLINE void *GLU_gset_iterator_get_key(GSetIterator *gsi)
{
	return GLU_ghash_iterator_get_key((GHashIterator *)gsi);
}
LOOM_INLINE void GLU_gset_iterator_step(GSetIterator *gsi)
{
	GLU_ghash_iterator_step((GHashIterator *)gsi);
}
LOOM_INLINE bool GLU_gset_iterator_done(const GSetIterator *gsi)
{
	return GLU_ghash_iterator_done((const GHashIterator *)gsi);
}

#define GSET_ITER(gs_iter_, gset_) \
	for (GLU_gset_iterator_init(&gs_iter_, gset_); \
		 GLU_gset_iterator_done(&gs_iter_) == false; \
		 GLU_gset_iterator_step(&gs_iter_))

#define GSET_ITER_INDEX(gs_iter_, gset_, i_) \
	for (GLU_gset_iterator_init(&gs_iter_, gset_), i_ = 0; \
		 GLU_gset_iterator_done(&gs_iter_) == false; \
		 GLU_gset_iterator_step(&gs_iter_), i_++)

/** \} */

/* -------------------------------------------------------------------- */
/** \name GHash/GSet Macros
 * \{ */

#define GHASH_FOREACH_BEGIN(type, var, what) \
	do { \
		GHashIterator gh_iter##var; \
		GHASH_ITER (gh_iter##var, what) { \
			type var = (type)(GLU_ghash_iterator_get_value(&gh_iter##var));

#define GHASH_FOREACH_END() \
	} \
	} \
	while (0)

#define GSET_FOREACH_BEGIN(type, var, what) \
	do { \
		GSetIterator gh_iter##var; \
		GSET_ITER (gh_iter##var, what) { \
			type var = (type)(GLU_gset_iterator_get_key(&gh_iter##var));

#define GSET_FOREACH_END() \
	} \
	} \
	while (0)

/** \} */

/* -------------------------------------------------------------------- */
/** \name GHash/GSet Utils
 *
 * Defined in `ghash_utils.c`
 * \{ */

/**
 * Callbacks for GHash (`GLU_ghashutil_*`)
 *
 * \note `_p` suffix denotes void pointer arg,
 * so we can have functions that take correctly typed args too.
 */

unsigned int GLU_ghashutil_ptrhash(const void *key);

bool GLU_ghashutil_ptrcmp(const void *a, const void *b);

/**
 * This function implements the widely used `djb`. The 32 bit unsigned hash
 * value starts at 5381 and for each byte 'c' in the string, is updated: `hash =
 * hash * 33 + c`. This function uses the signed value of each byte.
 *
 * NOTE: this is the same hash method that glib 2.34.0 uses.
 */
unsigned int GLU_ghashutil_strhash_n(const char *key, unsigned int n);

#define GLU_ghash_strhash(key) (GLU_ghashutil_strhash_p(key))

unsigned int GLU_ghashutil_strhash_p(const void *ptr);
unsigned int GLU_ghashutil_strhash_p_murmur(const void *ptr);

bool GLU_ghashutil_strcmp(const void *a, const void *b);

#define GLU_ghash_inthash(key) (GLU_ghashutil_uinthash(unsigned int) key)
unsigned int GLU_ghashutil_uinthash(unsigned int key);
unsigned int GLU_ghashutil_inthash_p(const void *ptr);
unsigned int GLU_ghashutil_inthash_p_murmur(const void *ptr);
unsigned int GLU_ghashutil_inthash_p_simple(const void *ptr);

bool GLU_ghashutil_intcmp(const void *a, const void *b);

unsigned int GLU_ghashutil_combine_hash(unsigned int hash_a,
										unsigned int hash_b);

unsigned int GLU_ghashutil_uinthash_v4(const unsigned int key[4]);
#define GLU_ghashutil_inthash_v4(key) \
	(GLU_ghashutil_uinthash_v4((const unsigned int *)key))
#define GLU_ghashutil_inthash_v4_p ((GSetHashFP)GLU_ghashutil_uinthash_v4)
#define GLU_ghashutil_uinthash_v4_p ((GSetHashFP)GLU_ghashutil_uinthash_v4)
unsigned int GLU_ghashutil_uinthash_v4_murmur(const unsigned int key[4]);
#define GLU_ghashutil_inthash_v4_murmur(key) \
	(GLU_ghashutil_uinthash_v4_murmur((const unsigned int *)key))
#define GLU_ghashutil_inthash_v4_p_murmur \
	((GSetHashFP)GLU_ghashutil_uinthash_v4_murmur)
#define GLU_ghashutil_uinthash_v4_p_murmur \
	((GSetHashFP)GLU_ghashutil_uinthash_v4_murmur)
bool GLU_ghashutil_uinthash_v4_cmp(const void *a, const void *b);
#define GLU_ghashutil_inthash_v4_cmp GLU_ghashutil_uinthash_v4_cmp

typedef struct GHashPair {
	const void *first;
	const void *second;
} GHashPair;

GHashPair *GLU_ghashutil_pairalloc(const void *first, const void *second);
unsigned int GLU_ghashutil_pairhash(const void *ptr);
bool GLU_ghashutil_paircmp(const void *a, const void *b);
void GLU_ghashutil_pairfree(void *ptr);

GHash *GLU_ghash_ptr_new_ex(const char *info, unsigned int nentries_reserve);
GHash *GLU_ghash_ptr_new(const char *info);
GHash *GLU_ghash_str_new_ex(const char *info, unsigned int nentries_reserve);
GHash *GLU_ghash_str_new(const char *info);
GHash *GLU_ghash_int_new_ex(const char *info, unsigned int nentries_reserve);
GHash *GLU_ghash_int_new(const char *info);
GHash *GLU_ghash_pair_new_ex(const char *info, unsigned int nentries_reserve);
GHash *GLU_ghash_pair_new(const char *info);

GSet *GLU_gset_ptr_new_ex(const char *info, unsigned int nentries_reserve);
GSet *GLU_gset_ptr_new(const char *info);
GSet *GLU_gset_str_new_ex(const char *info, unsigned int nentries_reserve);
GSet *GLU_gset_str_new(const char *info);
GSet *GLU_gset_pair_new_ex(const char *info, unsigned int nentries_reserve);
GSet *GLU_gset_pair_new(const char *info);
GSet *GLU_gset_int_new_ex(const char *info, unsigned int nentries_reserve);
GSet *GLU_gset_int_new(const char *info);

/** \} */

#ifdef __cplusplus
}
#endif
