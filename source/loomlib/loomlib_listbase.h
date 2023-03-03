#pragma once

#include "makesdna/dna_listbase.h"

#include "loomlib_utildefines.h"
#include "loomlib_compiler.h"

/**
 * A list of links (struct Link) which can be iterated through.
 * The main limitation of this data structure is that each element can only be in one list at a time.
 * This is useful for storing lists of objects which are not expected to be in multiple lists at once.
 * Such as active objects in a scene, or objects in a group.
 *
 * Allowing us to keep track of 'alive' objects in our program/scene.
 */

struct ListBase;
struct Link;

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * Iterates through the items of the specified \a listbase to find and return the position of 
 * the specified \a vlink within, numbering from 0. It might return -1 if the specifeid link 
 * was not found within the \a listbase.
 */
int GLU_findindex ( const struct ListBase *listbase , const void *vlink );

/**
 * Iterates through the items of the specified \a listbase to find and return the 0-based index 
 * of the first element of the listbase which contains the specified null-terminated string at the specified 
 * offset, or -1 if no such element was found.
 */
int GLU_findstringindex ( const struct ListBase *listbase , const char *id , int offset );

/** 
 * Returns a ListBase representing the entire list the specified link is currently stored in.
 */
ListBase GLU_listbase_from_link ( struct Link *link );

/* -------------------------------------------------------------------- */
/* \name Find Forwards
 * \{ */

/** 
 * Returns the element in the \a listbase at the specified index \a number, numbering from 0.
 */
void *GLU_findlink ( const struct ListBase *listbase , int number );

/** 
 * Returns teh n-th element after \a link, numbering from 0.
 */
void *GLU_findlinkfrom ( struct Link *start , int number );

/** 
 * Finds the first element of \a listbase which contains the specified null-terminated string \a id 
 * at the specified \a offset and returns the link, returning NULL if not found.
 */
void *GLU_findstring ( const struct ListBase *listbase , const char *id , int offset );

/**
 * Finds the first element of \a listbase which contains a pointer to the
 * null-terminated string \a id at the specified offset, returning NULL if not found.
 */
void *GLU_findstring_ptr ( const struct ListBase *listbase , const char *id , int offset );

/**
 * Find the first element of \a listbase which contains the specified \a ptr pointer value at 
 * the specified \a offset, returning NULL if no such element was found.
 */
void *GLU_findptr ( const struct ListBase *listbase , const void *ptr , int offset );

/** 
 * Find the frst element of \a listbase which contains the specified \a bytes of length \a size 
 * at the specified \a offset from the beginning of the element, returning NULL if no such element was found.
 */
void *GLU_listbase_bytes_find ( const struct ListBase *listbase , const void *bytes , size_t size , int offset );

/**
 * Find the first item in the list that matches the given string, or the given index as a fallback.
 * \note The string is only used if non-NULL and non-empty.
 * \return The item if found, otherwise NULL.
 */
void *GLU_listbase_string_or_index_find ( const struct ListBase *listbase , const char *string , size_t offset , int index );

/** \} */

/* -------------------------------------------------------------------- */
/* \name Find Backwards
 * \{ */

/**
 * Iterates through the items of the specified \a listbase in reverse order, to find and return 
 * the element in the \a number position (from the end), numbering from 0.
 */
void *GLU_rfindlink ( const struct ListBase *listbase , int number );

/**
 * Iterates through the items of the specified \a listbase in reverse order starting from \a start 
 * and returns the item after \a number elements, numbering from 0.
 */
void *GLU_rfindlinkfrom ( struct Link *start , int number );

/** 
 * Finds the last element of the \a listbase which contains the specified null-terminated string \a id 
 * at the specifeid offset, returning NULL if not found.
 */
void *GLU_rfindstring ( const struct ListBase *listbase , const char *id , int offset );

/** 
 * Find the last element of listbase which contains the specified pointer value at the specified offset, 
 * returning NULL if no such element was found.
 */
void *GLU_rfindptr ( const struct ListBase *listbase , const void *ptr , int offset );

/**
 * Finds the last element of the specified \a listbase which contains the specified \a bytes of length \a size
 * at the specified offset \a offset, returning NULL if no such element exists in the \a listbase.
 */
void *GLU_listbase_bytes_rfind ( const struct ListBase *listbase , const void *bytes , size_t size , int offset );

/** \} */

/**
 * Removes and disposes of the entire contents of \a listbase using guardedalloc.
 */
void GLU_freelistN ( struct ListBase *listbase );

/**
 * Appends \a vlink ( assumed to begin with a Link ) onto listbase.
 */
void GLU_addtail ( struct ListBase *listbase , void *vlink );

/**
 * Removes \a vlink from \a listbase. Assumes it is linked into there!
 */
void GLU_remlink ( struct ListBase *listbase , void *vlink );

/**
 * Checks that \a vlink is linked into \a listbase, removing it from there if so.
 */
bool GLU_remlink_safe ( struct ListBase *listbase , void *vlink );

/**
 * Removes the head from \a listbase and returns it.
 */
void *GLU_pophead ( struct ListBase *listbase );

/**
 * Removes the tail from \a listbase and returns it.
 */
void *GLU_poptail ( struct ListBase *listbase );

/** 
 * Prepends \a vlink ( assumed to begin with a Link ) onto \a listbase.
 */
void GLU_addhead ( struct ListBase *listbase , void *vlink );

/**
 * Inserts \a vlink ( assumed to begin with a Link ) into \a listbase after \a vprevlink.
 */
void GLU_insertlinkafter ( struct ListBase *listbase , void *vprevlink , void *vnewlink );

/**
 * Inserts \a vlink ( assumed to begin with a Link ) into \a listbase before \a vnextlink.
 */
void GLU_insertlinkbefore ( struct ListBase *listbase , void *vnextlink , void *vnewlink );

/**
 * Insert a link in place of another, without changing its position in the list.
 *
 * Puts \a vnewlink in the position of \a vreplacelink`, removing \a vreplacelink.
 * - \a vreplacelink *must* be in the list.
 * - \a vnewlink *must not* be in the list.
 */
void GLU_insertlinkreplace ( ListBase *listbase , void *vreplacelink , void *vnewlink );

/**
 * Reinsert \a vlink relative to its current position but offset by \a step. Doesn't move
 * item if new position would exceed list (could optionally move to head/tail).
 *
 * \param step: Absolute value defines step size, sign defines direction. E.g pass -1
 *              to move \a vlink before previous, or 1 to move behind next.
 * \return If position of \a vlink has changed.
 */
bool GLU_listbase_link_move ( ListBase *listbase , void *vlink , int step );

/**
 * Move the link at the index \a from to the position at index \a to.
 *
 * \return If the move was successful.
 */
bool GLU_listbase_move_index ( ListBase *listbase , int from , int to );

/**
 * Removes and disposes of the entire contents of listbase using direct free(3).
 */
void GLU_freelist ( struct ListBase *listbase );

/**
 * Returns the number of elements in \a listbase, up until (and including count_max).
 * \nore Use to avoid redundant looping.
 */
int GLU_listbase_count_at_most ( const struct ListBase *listbase , int count_max );

/**
 * Returns the number of elements in \a listbase.
 */
int GLU_listbase_count ( const struct ListBase *listbase );

/**
 * Swaps \a vlinka and \a vlinkb in the list. Assumes they are both already in the list!
 */
void GLU_listbase_swaplinks ( struct ListBase *listbase , void *vlinka , void *vlinkb );

/**
 * Swaps \a vlinka and \a vlinkb from their respective lists.
 * Assumes they are both already in their \a listbasea!
 */
void GLU_listbases_swaplinks ( struct ListBase *listbasea , struct ListBase *listbaseb , void *vlinka , void *vlinkb );

/**
 * Moves the entire contents of \a src onto the end of \a dst.
 */
void GLU_movelisttolist ( struct ListBase *dst , struct ListBase *src );

/**
 * Moves the entire contents of \a src at the beginning of \a dst.
 */
void GLU_movelisttolist_reverse ( struct ListBase *dst , struct ListBase *src );

/**
 * Sets dst to a duplicate of the entire contents of src. dst may be the same as src.
 */
void GLU_duplicatelist ( struct ListBase *dst , const struct ListBase *src );

/**
 * Reverse the order of the items in \a listbase
 */
void GLU_listbase_reverse ( struct ListBase *listbase );

/**
 * \param vlink: Link to make first.
 */
void GLU_listbase_rotate_first ( struct ListBase *listbase , void *vlink );

/**
 * \param vlink: Link to make last.
 */
void GLU_listbase_rotate_last ( struct ListBase *listbase , void *vlink );

LOOM_INLINE bool GLU_listbase_is_single ( const struct ListBase *listbase ) {
	return ( listbase->first && listbase->first == listbase->last );
}

LOOM_INLINE bool GLU_listbase_is_empty ( const struct ListBase *listbase ) {
	return ( listbase->first == ( void * ) 0 );
}

LOOM_INLINE void GLU_listbase_clear ( struct ListBase *listbase ) {
	listbase->first = listbase->last = ( void * ) 0;
}

/**
 * Equality check for ListBase.
 *
 * This only shallowly compares the ListBase itself (so the first/last
 * pointers), and does not do any equality checks on the list items.
 */
LOOM_INLINE bool GLU_listbase_equal ( const struct ListBase *a , const struct ListBase *b ) {
	if ( a == NULL ) {
		return b == NULL;
	}
	if ( b == NULL ) {
		return false;
	}
	return a->first == b->first && a->last == b->last;
}

/**
 * Create a generic list node containing link to provided data.
 */
struct LinkData *GLU_genericNodeN ( void *data );

/**
 * Does a full loop on the list, with any value acting as first
 * (handy for cycling items)
 *
 * \code{.c}
 *
 * LISTBASE_CIRCULAR_FORWARD_BEGIN(type, listbase, item, item_init)
 * {
 *     ...operate on marker...
 * }
 * LISTBASE_CIRCULAR_FORWARD_END (type, listbase, item, item_init);
 *
 * \endcode
 */
#define LISTBASE_CIRCULAR_FORWARD_BEGIN(type, lb, lb_iter, lb_init) \
  if ((lb)->first && (lb_init || (lb_init = (type)(lb)->first))) { \
    lb_iter = (type)(lb_init); \
    do {
#define LISTBASE_CIRCULAR_FORWARD_END(type, lb, lb_iter, lb_init) \
  } \
  while ((lb_iter = (lb_iter)->next ? (type)(lb_iter)->next : (type)(lb)->first), \
         (lb_iter != lb_init)) \
    ; \
  } \
  ((void)0)

#define LISTBASE_CIRCULAR_BACKWARD_BEGIN(type, lb, lb_iter, lb_init) \
  if ((lb)->last && (lb_init || (lb_init = (type)(lb)->last))) { \
    lb_iter = lb_init; \
    do {
#define LISTBASE_CIRCULAR_BACKWARD_END(type, lb, lb_iter, lb_init) \
  } \
  while ((lb_iter = (lb_iter)->prev ? (lb_iter)->prev : (type)(lb)->last), (lb_iter != lb_init)) \
    ; \
  } \
  ((void)0)

#define LISTBASE_FOREACH(type, var, list) \
  for (type var = (type)((list)->first); var != NULL; var = (type)(((Link *)(var))->next))

/**
 * A version of #LISTBASE_FOREACH that supports incrementing an index variable at every step.
 * Including this in the macro helps prevent mistakes where "continue" mistakenly skips the
 * incrementation.
 */
#define LISTBASE_FOREACH_INDEX(type, var, list, index_var) \
  for (type var = (((void)(index_var = 0)), (type)((list)->first)); var != NULL; \
       var = (type)(((Link *)(var))->next), index_var++)

#define LISTBASE_FOREACH_BACKWARD(type, var, list) \
  for (type var = (type)((list)->last); var != NULL; var = (type)(((Link *)(var))->prev))

/**
 * A version of #LISTBASE_FOREACH that supports removing the item we're looping over.
 */
#define LISTBASE_FOREACH_MUTABLE(type, var, list) \
  for (type var = (type)((list)->first), *var##_iter_next; \
       ((var != NULL) ? ((void)(var##_iter_next = (type)(((Link *)(var))->next)), 1) : 0); \
       var = var##_iter_next)

/**
 * A version of #LISTBASE_FOREACH_BACKWARD that supports removing the item we're looping over.
 */
#define LISTBASE_FOREACH_BACKWARD_MUTABLE(type, var, list) \
  for (type var = (type)((list)->last), *var##_iter_prev; \
       ((var != NULL) ? ((void)(var##_iter_prev = (type)(((Link *)(var))->prev)), 1) : 0); \
       var = var##_iter_prev)

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
LOOM_INLINE bool operator==( const ListBase &a , const ListBase &b ) {
	return GLU_listbase_equal ( &a , &b );
}
#endif
