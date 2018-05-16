
/**
 * lib_def.h
 * abstraction of some apis from libC
 *
 */

#ifndef __LIB_DEF_H__
#define __LIB_DEF_H__

/** 
 * \addtogroup PAL
 * @{
 * \addtogroup LIB
 * @{
 */
/** convert a macro variable into a string */
#define  STRINGIFY(x)  #x
#define  SFMT(MAX)     "%-" STRINGIFY(MAX) "s"

/**
 * \addtogroup error_codes
 * @{
 */
#define MEM_SIZE_ZERO             -1
#define MEM_POOL_FULL             -2
#define MEM_ADDR_OUT_OF_RANGE     -3
#define MEM_ADDR_NOT_ALIGNED      -4
#define MEM_POOL_INVALID          -5
#define MEM_POOL_CORRUPTED        -6
#define MEM_SIZE_TOO_BIG          -7
/**@}*/

/**
 * \addtogroup pool
 * @{
 */
#define PAGE_DEF_ALLOC_GLOBAL     1
#define PAGE_DEF_ALLOC_MAPPED     2
/**@}*/

/**@} PAL */
/**@} LIB */
#endif /*__LIB_DEF_H__*/
