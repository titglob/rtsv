#ifndef LIB_LIST_H
#define LIB_LIST_H

#include <cpu.h>

/** 
 * \addtogroup PAL
 * @{
 * \addtogroup LIB
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif
/**
 * \addtogroup list
 * @{
 */
/**
 * Simple doubly linked list node.
 * Insert it in your structure, then you can create a list of it.
 */

typedef struct list_node
{
   struct list_node *pnext;
   struct list_node *pprev;
}
list_node_t ;

/**
 * Use his when you want to declare an use a list inside your code
 */
#define list_node_declare(name) struct list_node name = { &(name), &(name) }
void    list_init(struct list_node *list);

int     list_count(struct list_node *plist);

void    list_insert_after(struct list_node *node, struct list_node *head);

void    list_insert_before(struct list_node *node, struct list_node *head);

void    list_delete(struct list_node *entry);

#define list_add_head(node, list)  list_insert_after(node, list)

#define list_add_tail(node, list)  list_insert_before(node, list)

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int list_empty(const struct list_node *head)
{
   return head->pnext == head;
}

/**
 * Get offset of a member
 */
#define offset_of(type, member) (((unsigned long) &((type *)1)->member) - 1)

/**
 * Casts a member of a structure out to the containing structure
 * @param ptr        the pointer to the member.
 * @param type       the type of the container struct this is embedded in.
 * @param member     the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({                \
        typeof( ((type *)1)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offset_of(type,member) );})

/**
 * Casts a member of a structure out to the containing structure
 * @param ptr        the pointer to the specialized object.
 * @param type       type of the generalized object.
 * @param member     member of the specialized object in case general
 *                   object is no included at the beginning.
 */
#define specialization_of(ptr, type, member) ({            \
        (type *)(&(ptr)->member) ;})

/**
 * get the struct for this entry
 */
#define list_entry(ptr, type, member) \
   container_of(ptr, type, member)

/**
 * Iterate over a list. WARNING: do not remove any node while doing the loop !
 * @pos:	the &struct ListNode to use as a loop cursor.
 * @head:	the head for your list.
 */
#define list_for_each(pos, head) \
   for (pos = (head)->pnext; pos != (head); pos = pos->pnext)

/**
 * Iterate over a list in the reverse order. WARNING: do not remove any node while doing the loop !
 * @pos:	the &struct ListNode to use as a loop cursor.
 * @head:	the head for your list.
 */
#define list_for_each_rev(pos, head) \
   for (pos = (head)->pprev; pos != (head); pos = pos->pprev)

/**
 * iterate over a list safe against removal of list entry
 * @pos:	the &struct ListNode to use as a loop cursor.
 * @n:		another &struct ListNode to use as temporary storage
 * @head:	the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->pnext, n = pos->pnext; pos != (head); \
		pos = n, n = pos->pnext)

/**@}*/
#ifdef __cplusplus
}
#endif

/* @} LIB
 * @} PAL */

#endif

