#ifndef LIB_MEMSET_H
#define LIB_MEMSET_H

#include <cpu.h>

#include <lib_def.h>

/** 
 * \addtogroup PAL
 * @{
 * \addtogroup LIB
 * @{
 * \addtogroup mem
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Init part of memory with value
 * @param[in] s first byte position
 * @param[in] c value to use to initialize each byte
 * @param[in] n number of bytes to initialize with c
 * @return none
 */
void         mem_set(void *s, int c, size_t n);

/**
 * copy part of memory
 * @param[in,out] dest -ination already allocated
 * @param[in] src data source
 * @param[in] n size of dest
 * @return ptr towards dest
 */
void       * mem_cpy(void *dest, const void *src, size_t n);

/**
 * Move part of memory
 * @param[in,out] dest -ination already allocated
 * @param[in] src data source
 * @param[in] n size of dest
 * @return ptr towards dest
 */

void       * mem_move(void *dest, const void *src, size_t n);

/**
 * compare part of memory
 * @param[in] s1
 * @param[in] s2
 * @param[in] n number of bytes to compare
 * @return s1-s2 (>,<,= 0)
 */

int          mem_cmp(const void *s1, const void *s2, size_t n);

#ifdef __cplusplus
}
#endif

/**@} mem */
/**@} LIB */
/**@} PAL */

#endif
