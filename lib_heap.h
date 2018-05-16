#ifndef LIB_MEM_TEMPLATE_H
#define LIB_MEM_TEMPLATE_H

#include <cpu.h>

#include <lib_def.h>

/** 
 * \addtogroup PAL
 * @{
 * \addtogroup LIB
 * @{
 */

/**
 * \addtogroup heap
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Stuff to initialize the heap (for memory mapped applications mainly)
 * @return 0 (OK) or -1
 */
int          heap_init(void);
/**
 * release memory initialized via @ref heap_init
 * @pre @ref heap_init
 * @return none
 */
void         heap_end(void);

/**
 * allocation of bytes
 * @param[in] bytes number of bytes to allocate
 * @return pointer towards allocated memory
 */
void*        heap_alloc(size_t bytes);
/**
 * free allocated memory 
 * @pre @ref heap_alloc, @ref heap_realloc
 * @param[in] ptr on allocated memory
 * @return pointer towards freed memory
 */

void         heap_free(void* ptr);

/**
 * reallocation of bytes
 * @pre @ref heap_alloc, @ref heap_realloc
 * @param[in] ptr already allocated before
 * @param[in] size new number of bytes to allocate
 * @return pointer towards allocated memory
 */
void*        heap_realloc(void* ptr, size_t size);

/**
 * allocate memory aligned
 * @pre @ref heap_init
 * @param[in] align the memory address is a multiple of align
 * @param[in] bytes nbr of bytes to allocate
 * @return ptr towards allocated memory
 */
void*        heap_align(size_t align, size_t bytes);

/**
 * Dump all memory
 * @return none
 */
void         heap_dump(void);

/**
 * test memory
 * @return none
 */
int          heap_check();

#ifdef __cplusplus
}
#endif

/**@} heap */
/**@} LIB  */
/**@} PAL  */

#endif
