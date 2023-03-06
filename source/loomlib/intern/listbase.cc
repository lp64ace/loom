#include "guardedalloc/mem_guardedalloc.h"

#include "loomlib/loomlib_assert.h"
#include "loomlib/loomlib_listbase.h"

#include <stdlib.h>
#include <string.h>

void GLU_movelisttolist(ListBase *dst, ListBase *src)
{
	if (src->first == nullptr) {
		return;
	}

	if (dst->first == nullptr) {
		dst->first = src->first;
		dst->last = src->last;
	}
	else {
		((Link *)dst->last)->next = static_cast<Link *>(src->first);
		((Link *)src->first)->prev = static_cast<Link *>(dst->last);
		dst->last = src->last;
	}
	src->first = src->last = nullptr;
}

void GLU_movelisttolist_reverse(ListBase *dst, ListBase *src)
{
	if (src->first == nullptr) {
		return;
	}

	if (dst->first == nullptr) {
		dst->first = src->first;
		dst->last = src->last;
	}
	else {
		((Link *)src->last)->next = static_cast<Link *>(dst->first);
		((Link *)dst->first)->prev = static_cast<Link *>(src->last);
		dst->first = src->first;
	}
	src->first = src->last = nullptr;
}

void GLU_addhead(ListBase *listbase, void *vlink)
{
	Link *link = static_cast<Link *>(vlink);

	if (link == nullptr) {
		return;
	}

	link->next = static_cast<Link *>(listbase->first);
	link->prev = nullptr;

	if (listbase->first) {
		((Link *)listbase->first)->prev = link;
	}
	if (listbase->last == nullptr) {
		listbase->last = link;
	}
	listbase->first = link;
}

void GLU_addtail(ListBase *listbase, void *vlink)
{
	Link *link = static_cast<Link *>(vlink);

	if (link == nullptr) {
		return;
	}

	link->next = nullptr;
	link->prev = static_cast<Link *>(listbase->last);

	if (listbase->last) {
		((Link *)listbase->last)->next = link;
	}
	if (listbase->first == nullptr) {
		listbase->first = link;
	}
	listbase->last = link;
}

void GLU_remlink(ListBase *listbase, void *vlink)
{
	Link *link = static_cast<Link *>(vlink);

	if (link == nullptr) {
		return;
	}

	if (link->next) {
		link->next->prev = link->prev;
	}
	if (link->prev) {
		link->prev->next = link->next;
	}

	if (listbase->last == link) {
		listbase->last = link->prev;
	}
	if (listbase->first == link) {
		listbase->first = link->next;
	}
}

bool GLU_remlink_safe(ListBase *listbase, void *vlink)
{
	if (GLU_findindex(listbase, vlink) != -1) {
		GLU_remlink(listbase, vlink);
		return true;
	}

	return false;
}

void GLU_listbase_swaplinks(ListBase *listbase, void *vlinka, void *vlinkb)
{
	Link *linka = static_cast<Link *>(vlinka);
	Link *linkb = static_cast<Link *>(vlinkb);

	if (!linka || !linkb) {
		return;
	}

	if (linkb->next == linka) {
		std::swap(linka, linkb);
	}

	if (linka->next == linkb) {
		linka->next = linkb->next;
		linkb->prev = linka->prev;
		linka->prev = linkb;
		linkb->next = linka;
	}
	else { /* Non-contiguous items, we can safely swap. */
		std::swap(linka->prev, linkb->prev);
		std::swap(linka->next, linkb->next);
	}

	/* Update neighbors of linka and linkb. */
	if (linka->prev) {
		linka->prev->next = linka;
	}
	if (linka->next) {
		linka->next->prev = linka;
	}
	if (linkb->prev) {
		linkb->prev->next = linkb;
	}
	if (linkb->next) {
		linkb->next->prev = linkb;
	}

	if (listbase->last == linka) {
		listbase->last = linkb;
	}
	else if (listbase->last == linkb) {
		listbase->last = linka;
	}

	if (listbase->first == linka) {
		listbase->first = linkb;
	}
	else if (listbase->first == linkb) {
		listbase->first = linka;
	}
}

void BLI_listbases_swaplinks(ListBase *listbasea,
							 ListBase *listbaseb,
							 void *vlinka,
							 void *vlinkb)
{
	Link *linka = static_cast<Link *>(vlinka);
	Link *linkb = static_cast<Link *>(vlinkb);
	Link linkc = {nullptr};

	if (!linka || !linkb) {
		return;
	}

/* The reference to `linkc` assigns nullptr, not a dangling pointer so it can be
 * ignored. */
#if defined(__GNUC__) && \
	(__GNUC__ * 100 + __GNUC_MINOR__) >= 1201 /* gcc12.1+ only */
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wdangling-pointer"
#endif

	/* Temporary link to use as placeholder of the links positions */
	GLU_insertlinkafter(listbasea, linka, &linkc);

#if defined(__GNUC__) && \
	(__GNUC__ * 100 + __GNUC_MINOR__) >= 1201 /* gcc12.1+ only */
#	pragma GCC diagnostic pop
#endif

	/* Bring linka into linkb position */
	GLU_remlink(listbasea, linka);
	GLU_insertlinkafter(listbaseb, linkb, linka);

	/* Bring linkb into linka position */
	GLU_remlink(listbaseb, linkb);
	GLU_insertlinkafter(listbasea, &linkc, linkb);

	/* Remove temporary link */
	GLU_remlink(listbasea, &linkc);
}

void *GLU_pophead(ListBase *listbase)
{
	Link *link;
	if ((link = static_cast<Link *>(listbase->first))) {
		GLU_remlink(listbase, link);
	}
	return link;
}

void *GLU_poptail(ListBase *listbase)
{
	Link *link;
	if ((link = static_cast<Link *>(listbase->last))) {
		GLU_remlink(listbase, link);
	}
	return link;
}

void BLI_freelinkN(ListBase *listbase, void *vlink)
{
	Link *link = static_cast<Link *>(vlink);

	if (link == nullptr) {
		return;
	}

	GLU_remlink(listbase, link);
	MEM_freeN(link);
}

/**
 * Assigns all #Link.prev pointers from #Link.next
 */
static void listbase_double_from_single(Link *iter, ListBase *listbase)
{
	Link *prev = nullptr;
	listbase->first = iter;
	do {
		iter->prev = prev;
		prev = iter;
	} while ((iter = iter->next));
	listbase->last = prev;
}

void GLU_insertlinkafter(ListBase *listbase, void *vprevlink, void *vnewlink)
{
	Link *prevlink = static_cast<Link *>(vprevlink);
	Link *newlink = static_cast<Link *>(vnewlink);

	/* newlink before nextlink */
	if (newlink == nullptr) {
		return;
	}

	/* empty list */
	if (listbase->first == nullptr) {
		listbase->first = newlink;
		listbase->last = newlink;
		return;
	}

	/* insert at head of list */
	if (prevlink == nullptr) {
		newlink->prev = nullptr;
		newlink->next = static_cast<Link *>(listbase->first);
		newlink->next->prev = newlink;
		listbase->first = newlink;
		return;
	}

	/* at end of list */
	if (listbase->last == prevlink) {
		listbase->last = newlink;
	}

	newlink->next = prevlink->next;
	newlink->prev = prevlink;
	prevlink->next = newlink;
	if (newlink->next) {
		newlink->next->prev = newlink;
	}
}

void GLU_insertlinkbefore(ListBase *listbase, void *vnextlink, void *vnewlink)
{
	Link *nextlink = static_cast<Link *>(vnextlink);
	Link *newlink = static_cast<Link *>(vnewlink);

	/* newlink before nextlink */
	if (newlink == nullptr) {
		return;
	}

	/* empty list */
	if (listbase->first == nullptr) {
		listbase->first = newlink;
		listbase->last = newlink;
		return;
	}

	/* insert at end of list */
	if (nextlink == nullptr) {
		newlink->prev = static_cast<Link *>(listbase->last);
		newlink->next = nullptr;
		((Link *)listbase->last)->next = newlink;
		listbase->last = newlink;
		return;
	}

	/* at beginning of list */
	if (listbase->first == nextlink) {
		listbase->first = newlink;
	}

	newlink->next = nextlink;
	newlink->prev = nextlink->prev;
	nextlink->prev = newlink;
	if (newlink->prev) {
		newlink->prev->next = newlink;
	}
}

void GLU_insertlinkreplace(ListBase *listbase,
						   void *vreplacelink,
						   void *vnewlink)
{
	Link *l_old = static_cast<Link *>(vreplacelink);
	Link *l_new = static_cast<Link *>(vnewlink);

	/* update adjacent links */
	if (l_old->next != nullptr) {
		l_old->next->prev = l_new;
	}
	if (l_old->prev != nullptr) {
		l_old->prev->next = l_new;
	}

	/* set direct links */
	l_new->next = l_old->next;
	l_new->prev = l_old->prev;

	/* update list */
	if (listbase->first == l_old) {
		listbase->first = l_new;
	}
	if (listbase->last == l_old) {
		listbase->last = l_new;
	}
}

bool GLU_listbase_link_move(ListBase *listbase, void *vlink, int step)
{
	Link *link = static_cast<Link *>(vlink);
	Link *hook = link;
	const bool is_up = step < 0;

	if (step == 0) {
		return false;
	}
	LOOM_assert(GLU_findindex(listbase, link) != -1);

	/* find link to insert before/after */
	const int abs_step = abs(step);
	for (int i = 0; i < abs_step; i++) {
		hook = is_up ? hook->prev : hook->next;
		if (!hook) {
			return false;
		}
	}

	/* reinsert link */
	GLU_remlink(listbase, vlink);
	if (is_up) {
		GLU_insertlinkbefore(listbase, hook, vlink);
	}
	else {
		GLU_insertlinkafter(listbase, hook, vlink);
	}
	return true;
}

bool GLU_listbase_move_index(ListBase *listbase, int from, int to)
{
	if (from == to) {
		return false;
	}

	/* Find the link to move. */
	void *link = GLU_findlink(listbase, from);

	if (!link) {
		return false;
	}

	return GLU_listbase_link_move(listbase, link, to - from);
}

void GLU_freelist(ListBase *listbase)
{
	Link *link, *next;

	link = static_cast<Link *>(listbase->first);
	while (link) {
		next = link->next;
		free(link);
		link = next;
	}

	GLU_listbase_clear(listbase);
}

void GLU_freelistN(ListBase *listbase)
{
	Link *link, *next;

	link = static_cast<Link *>(listbase->first);
	while (link) {
		next = link->next;
		MEM_freeN(link);
		link = next;
	}

	GLU_listbase_clear(listbase);
}

int GLU_listbase_count_at_most(const ListBase *listbase, const int count_max)
{
	Link *link;
	int count = 0;

	for (link = static_cast<Link *>(listbase->first);
		 link && count != count_max;
		 link = link->next) {
		count++;
	}

	return count;
}

int GLU_listbase_count(const ListBase *listbase)
{
	Link *link;
	int count = 0;

	for (link = static_cast<Link *>(listbase->first); link; link = link->next) {
		count++;
	}

	return count;
}

void *GLU_findlink(const ListBase *listbase, int number)
{
	Link *link = nullptr;

	if (number >= 0) {
		link = static_cast<Link *>(listbase->first);
		while (link != nullptr && number != 0) {
			number--;
			link = link->next;
		}
	}

	return link;
}

void *GLU_rfindlink(const ListBase *listbase, int number)
{
	Link *link = nullptr;

	if (number >= 0) {
		link = static_cast<Link *>(listbase->last);
		while (link != nullptr && number != 0) {
			number--;
			link = link->prev;
		}
	}

	return link;
}

void *GLU_findlinkfrom(Link *start, int number)
{
	Link *link = nullptr;

	if (number >= 0) {
		link = start;
		while (link != nullptr && number != 0) {
			number--;
			link = link->next;
		}
	}

	return link;
}

int GLU_findindex(const ListBase *listbase, const void *vlink)
{
	Link *link = nullptr;
	int number = 0;

	if (vlink == nullptr) {
		return -1;
	}

	link = static_cast<Link *>(listbase->first);
	while (link) {
		if (link == vlink) {
			return number;
		}

		number++;
		link = link->next;
	}

	return -1;
}

void *GLU_findstring(const ListBase *listbase, const char *id, const int offset)
{
	Link *link = nullptr;
	const char *id_iter;

	if (id == nullptr) {
		return nullptr;
	}

	for (link = static_cast<Link *>(listbase->first); link; link = link->next) {
		id_iter = ((const char *)link) + offset;

		if (id[0] == id_iter[0] && STREQ(id, id_iter)) {
			return link;
		}
	}

	return nullptr;
}

void *GLU_rfindstring(const ListBase *listbase,
					  const char *id,
					  const int offset)
{
	/* Same as #BLI_findstring but find reverse. */

	Link *link = nullptr;
	const char *id_iter;

	for (link = static_cast<Link *>(listbase->last); link; link = link->prev) {
		id_iter = ((const char *)link) + offset;

		if (id[0] == id_iter[0] && STREQ(id, id_iter)) {
			return link;
		}
	}

	return nullptr;
}

void *GLU_findstring_ptr(const ListBase *listbase,
						 const char *id,
						 const int offset)
{
	Link *link = nullptr;
	const char *id_iter;

	for (link = static_cast<Link *>(listbase->first); link; link = link->next) {
		/* exact copy of BLI_findstring(), except for this line */
		id_iter = *((const char **)(((const char *)link) + offset));

		if (id[0] == id_iter[0] && STREQ(id, id_iter)) {
			return link;
		}
	}

	return nullptr;
}

void *GLU_rfindstring_ptr(const ListBase *listbase,
						  const char *id,
						  const int offset)
{
	/* Same as #BLI_findstring_ptr but find reverse. */

	Link *link = nullptr;
	const char *id_iter;

	for (link = static_cast<Link *>(listbase->last); link; link = link->prev) {
		/* exact copy of BLI_rfindstring(), except for this line */
		id_iter = *((const char **)(((const char *)link) + offset));

		if (id[0] == id_iter[0] && STREQ(id, id_iter)) {
			return link;
		}
	}

	return nullptr;
}

void *GLU_findptr(const ListBase *listbase, const void *ptr, const int offset)
{
	Link *link = nullptr;
	const void *ptr_iter;

	for (link = static_cast<Link *>(listbase->first); link; link = link->next) {
		/* exact copy of BLI_findstring(), except for this line */
		ptr_iter = *((const void **)(((const char *)link) + offset));

		if (ptr == ptr_iter) {
			return link;
		}
	}

	return nullptr;
}

void *GLU_rfindptr(const ListBase *listbase, const void *ptr, const int offset)
{
	/* Same as #BLI_findptr but find reverse. */

	Link *link = nullptr;
	const void *ptr_iter;

	for (link = static_cast<Link *>(listbase->last); link; link = link->prev) {
		/* exact copy of BLI_rfindstring(), except for this line */
		ptr_iter = *((const void **)(((const char *)link) + offset));

		if (ptr == ptr_iter) {
			return link;
		}
	}

	return nullptr;
}

void *GLU_listbase_bytes_find(const ListBase *listbase,
							  const void *bytes,
							  const size_t bytes_size,
							  const int offset)
{
	Link *link = nullptr;
	const void *ptr_iter;

	for (link = static_cast<Link *>(listbase->first); link; link = link->next) {
		ptr_iter = (const void *)(((const char *)link) + offset);

		if (memcmp(bytes, ptr_iter, bytes_size) == 0) {
			return link;
		}
	}

	return nullptr;
}

void *GLU_listbase_bytes_rfind(const ListBase *listbase,
							   const void *bytes,
							   const size_t bytes_size,
							   const int offset)
{
	/* Same as #BLI_listbase_bytes_find but find reverse. */

	Link *link = nullptr;
	const void *ptr_iter;

	for (link = static_cast<Link *>(listbase->last); link; link = link->prev) {
		ptr_iter = (const void *)(((const char *)link) + offset);

		if (memcmp(bytes, ptr_iter, bytes_size) == 0) {
			return link;
		}
	}

	return nullptr;
}

void *GLU_listbase_string_or_index_find(const ListBase *listbase,
										const char *string,
										const size_t string_offset,
										const int index)
{
	Link *link = nullptr;
	Link *link_at_index = nullptr;

	int index_iter;
	for (link = static_cast<Link *>(listbase->first), index_iter = 0; link;
		 link = link->next, index_iter++) {
		if (string != nullptr && string[0] != '\0') {
			const char *string_iter = ((const char *)link) + string_offset;

			if (string[0] == string_iter[0] && STREQ(string, string_iter)) {
				return link;
			}
		}
		if (index_iter == index) {
			link_at_index = link;
		}
	}
	return link_at_index;
}

int GLU_findstringindex(const ListBase *listbase,
						const char *id,
						const int offset)
{
	Link *link = nullptr;
	const char *id_iter;
	int i = 0;

	link = static_cast<Link *>(listbase->first);
	while (link) {
		id_iter = ((const char *)link) + offset;

		if (id[0] == id_iter[0] && STREQ(id, id_iter)) {
			return i;
		}
		i++;
		link = link->next;
	}

	return -1;
}

ListBase GLU_listbase_from_link(Link *some_link)
{
	ListBase list = {some_link, some_link};
	if (some_link == nullptr) {
		return list;
	}

	/* Find the first element. */
	while (((Link *)list.first)->prev != nullptr) {
		list.first = ((Link *)list.first)->prev;
	}

	/* Find the last element. */
	while (((Link *)list.last)->next != nullptr) {
		list.last = ((Link *)list.last)->next;
	}

	return list;
}

void GLU_duplicatelist(ListBase *dst, const ListBase *src)
{
	struct Link *dst_link, *src_link;

	/* in this order, to ensure it works if dst == src */
	src_link = static_cast<Link *>(src->first);
	dst->first = dst->last = nullptr;

	while (src_link) {
		dst_link = static_cast<Link *>(MEM_dupallocN(src_link));
		GLU_addtail(dst, dst_link);

		src_link = src_link->next;
	}
}

void GLU_listbase_reverse(ListBase *lb)
{
	struct Link *curr = static_cast<Link *>(lb->first);
	struct Link *prev = nullptr;
	struct Link *next = nullptr;
	while (curr) {
		next = curr->next;
		curr->next = prev;
		curr->prev = next;
		prev = curr;
		curr = next;
	}

	/* swap first/last */
	curr = static_cast<Link *>(lb->first);
	lb->first = lb->last;
	lb->last = curr;
}

void GLU_listbase_rotate_first(ListBase *lb, void *vlink)
{
	/* make circular */
	((Link *)lb->first)->prev = static_cast<Link *>(lb->last);
	((Link *)lb->last)->next = static_cast<Link *>(lb->first);

	lb->first = vlink;
	lb->last = ((Link *)vlink)->prev;

	((Link *)lb->first)->prev = nullptr;
	((Link *)lb->last)->next = nullptr;
}

void GLU_listbase_rotate_last(ListBase *lb, void *vlink)
{
	/* make circular */
	((Link *)lb->first)->prev = static_cast<Link *>(lb->last);
	((Link *)lb->last)->next = static_cast<Link *>(lb->first);

	lb->first = ((Link *)vlink)->next;
	lb->last = vlink;

	((Link *)lb->first)->prev = nullptr;
	((Link *)lb->last)->next = nullptr;
}

LinkData *GLU_genericNodeN(void *data)
{
	LinkData *ld;

	if (data == nullptr) {
		return nullptr;
	}

	/* create new link, and make it hold the given data */
	ld = MEM_cnew<LinkData>(__func__);
	ld->data = data;

	return ld;
}
