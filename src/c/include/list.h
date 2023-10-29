#ifndef __LIST_H
#define __LIST_H

#define __inline__ inline __attribute__((always_inline))

#define     OFFSET_OF(type, member) ((unsigned long long)((char *)&((struct type *)0)->member - (char *)0))
#define OFF OFFSET_OF

struct list_head {
	struct list_head *next, *prev;
};

static __inline__ void LIST_HEAD_INIT(struct list_head *list)
{
        list->next = list;
        list->prev = list;
}

#define LIST_HEAD_GLOBAL(name) { &(name), &(name) }

#define DLIST_HEAD(name) \
        struct list_head name = LIST_HEAD_GLOBAL(name)

static __inline__ void
LIST_ADD(struct list_head *newlist, struct list_head *head)
{
	struct list_head *next = head->next;
	struct list_head *prev = head;

	next->prev    = newlist;
	newlist->next = next;
	newlist->prev = prev;
	prev->next    = newlist;
}

static __inline__ void
LIST_DEL(struct list_head *entry)
{
	struct list_head *next = entry->next;
	struct list_head *prev = entry->prev;

	next->prev = prev;
	prev->next = next;
}

#define DLIST_ENTRY(ptr, type, member) \
        ((type *)((char *)(ptr)-(uint64_t)(&((type *)0)->member)))

#define DLIST_FOR_EACH_ENTRY(pos, head, member)                          \
        for (pos = DLIST_ENTRY((head)->next, typeof(*pos), member);      \
             &pos->member != (head);                                    \
             pos = DLIST_ENTRY(pos->member.next, typeof(*pos), member))

#endif
