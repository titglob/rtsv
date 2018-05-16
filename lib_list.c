
#include <lib_list.h>

/**
 * initialize a list if declared inside a structure using list_head
 * @param[in] plist
 */
void list_init(struct list_node *plist)
{
   plist->pnext = plist;
   plist->pprev = plist;
}

/**
 * count the number of element inside a list
 * @param[in] plist first element fro mthe list
 * @return number of elements
 */
int list_count(struct list_node *plist)
{
   int count = 0;
   struct list_node * pnode;
   list_for_each(pnode, plist)
   {
      count++;
   }
   return count;
}

/**
 * insert a new entry between two known consecutive entries. internal use only
 * @param[in] pnew
 * @param[in] pprev
 * @param[in] pnext
 * @return none
 */
static inline void list_insert(struct list_node *pnew, struct list_node *pprev, struct list_node *pnext)
{
   pnext->pprev = pnew;
   pnew->pnext = pnext;
   pnew->pprev = pprev;
   pprev->pnext = pnew;
}

/**
 * insert a new entry after the one given in argument
 * @param[in] pnew 
 * @param[in] pnode before pnew
 * @return none
 */
void list_insert_after(struct list_node *pnew, struct list_node *pnode)
{
   list_insert(pnew, pnode, pnode->pnext);
}

/**
 * insert a new entry before the one given in argument
 * @param[in] pnew 
 * @param[in] pnode after pnew
 * @return none
 */
void list_insert_before(struct list_node *pnew, struct list_node *pnode)
{
   list_insert(pnew, pnode->pprev, pnode);
}

/**
 * delete an entry from a list.
 * @param[in] entry
 * @return none
 */
void list_delete(struct list_node *entry)
{
   entry->pprev->pnext = entry->pnext;
   entry->pnext->pprev = entry->pprev;
}

