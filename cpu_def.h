#ifndef CPU_DEF_H
#define CPU_DEF_H

/** 
 * \addtogroup PAL
 * @{
 * \addtogroup CPU
 * @{
 */

#define   LEON        1
#define   SMALL_LEON  2
#define   LINUX_ARM   3
#define   TLM_LEON    4
#define   ARM         5
#define   LINUX_X86   6
#define   HD_LEON     7
#define   PV_LEON     8
#define   CAN_LEON    9
enum CpuType
{
   CPU_DEF_NONE        = 0,
   CPU_DEF_LEON        = LEON,
   CPU_DEF_SMALL_LEON  = SMALL_LEON,
   CPU_DEF_LINUX_ARM   = LINUX_ARM,
   CPU_DEF_TLM_LEON    = TLM_LEON,
   CPU_DEF_ARM         = ARM,
   CPU_DEF_LINUX_X86   = LINUX_X86,
   CPU_DEF_HD_LEON     = HD_LEON,
   CPU_DEF_PV_LEON     = PV_LEON,
   CPU_DEF_CAN_LEON    = CAN_LEON,
   CPU_DEF_CLIENT      = -1
};

typedef enum CpuType tCpuType;

/** 
 * \addtogroup endianness
 *  @{
 */
/**
 * cpu byte order, endianess
 * cpu.h must define CPU_BYTE_ORDER to one of this two values
 */
#define CPU_LITTLE_ENDIAN   1234
#define CPU_BIG_ENDIAN      4321

/**@}*/ /*endianness*/

/** 
 * \addtogroup stack
 * cpu.h must define CPU_STACK_GROWTH to one of this two values
  *  @{
 */

#define CPU_STACK_INCREASE  1
#define CPU_STACK_DECREASE -1

/**@}*/ /* stack */


/** 
 * \addtogroup simulation
 * time unit for simulation
 *  @{
 */
#define  T_SEC          0
#define  T_MS           1
#define  T_US           2
#define  T_NS           3

/**@}*/ /*simulation */

/** 
 * \addtogroup bus_access
 * @{
 */
#define BUS_ADDR_INVALID  -1
#define BUS_DATA_INVALID  -1
/**@}*/ /*bus_access */

/** 
 * \addtogroup implementation
 * This is the common declaration for all cpu irq handlers
 */
typedef void (*cpu_irq_handler)(int int_num, void * data);

/** 
 * \addtogroup implementation
 * This is the common declaration for all cpu irq polling routines.
 */
typedef int (*cpu_poll_handler)(int int_num, void * data, void * platform_context);

/**
 * handler for software traps
 */
typedef void (*cpu_trap_handler)(int trap_num);

/**
 * This structure is used to set or update information regarding an interrupt line
 */
struct cpu_irq_action
{
   cpu_irq_handler    irq_handler;
   void             * irq_data;
   cpu_poll_handler   poll_handler;
   void             * poll_data;
};

/** 
 * \addtogroup bus_access
 * @{
 */
#define BUS_DEF_ACCESS_MODE_MASK                 0x00000007
#define BUS_DEF_ACCESS_MODE_8BIT                 0x00000001
#define BUS_DEF_ACCESS_MODE_16BIT                0x00000002
#define BUS_DEF_ACCESS_MODE_32BIT                0x00000004
#define BUS_DEF_ACCESS_MODE_NO_SYNC              0x00000008
#define BUS_DEF_ACCESS_MODE_NO_ADDRESS_INCREMENT 0x00000010
#define BUS_DEF_ACCESS_MODE_ADDRESSING_MASK      0x000000E0
#define BUS_DEF_ACCESS_MODE_SEPARATE_ADDR_BUFFER 0x00000020
#define BUS_DEF_ACCESS_MODE_DMA                  0x00000040
#define BUS_DEF_ACCESS_MODE_START                0x00000080
#define BUS_DEF_ACCESS_MODE_BURST                0x00000100
#define BUS_DEF_ACCESS_MODE_24BIT                0x00000200
/**@}*/ /*bus_access */

/**@}*/ /*CPU*/
/**@}*/ /*PAL*/

#endif
