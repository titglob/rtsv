/**
 * \file cpu_linux.h
 *
 * This file regroup common definitions for linux-arm, linux-x86, pv-leon, tlm-leon and hd-leon procesors
 */

#ifndef __CPU_LINUX_H__
#define __CPU_LINUX_H__
#include <cpu_def.h>
#include <stdint.h>     // cpu types
#include <stddef.h>     // ptrdiff_t
#include <sys/types.h>  // ssize_t

// size of the data and address bus 
#if __x86_64__
#define CPU_DATA_SIZE       64
#define CPU_ADDR_SIZE       64
#else
#define CPU_DATA_SIZE       32
#define CPU_ADDR_SIZE       32
#endif
#define CPU_CHAR_SIZE       8

#define CPU_NB_REGISTERS    8
#define CPU_STACK_FRAME_SIZE  (CPU_NB_REGISTERS*sizeof(ustack_t))

/*
 * stack growth:
 */
#define CPU_STACK_GROWTH    CPU_STACK_DECREASE
#define CPU_STACK_ALIGN     8 // doule word aligned

/*
 * endianess
 */
#define CPU_BYTE_ORDER      CPU_LITTLE_ENDIAN

/*
 * bionic workaround
 */
#define SIGPTHEXIT SIGPWR

#ifdef __cplusplus
extern "C" {
#endif

/*
 * number of interrupt levels
 */
#define NB_IRQ_LEVELS       15

typedef  uint64_t           ustack_t;

/*
 * simulation
 */
#if CPU == TLM_LEON
void     sim_start(void);
void     sim_advance(int duration, int time_unit);
void     sim_stop(void);
#else
#define  sim_start()
#define  sim_advance(duration, time_unit)
#define  sim_stop()
#endif


/*
 * processor status register
 */
typedef  int cpu_sr;

#if LIB != BIONIC_LIB
/*
 * save the current processor context onto the stack.
 * This function is mainly used to initialize a new task.
 * Returns the adddress of the new stack afte saving.
 */
typedef struct cpu_context
{
	ucontext_t uctx;
	cpu_sr     psr;
}
cpu_context_t;
#endif


int      clkgen_get_cpu_frequency();

#define  cpu_psr_value(cpu_sr)  cpu_sr
#if CPU == HD_LEON
#define  cpu_reg_format         "%x"
#define  cpu_set_sp(n)  do { } while(0)
#define  cpu_set_fp(n)  do { } while(0)
#else
void     cpu_set_sp(uint32_t sp);
void     cpu_set_fp(uint32_t fp);
#define  cpu_reg_format         "%x"
#endif

void     cpu_ack_irq(int irq);

uint32_t cpu_get_sp();
uint32_t cpu_get_fp();
size_t   cpu_get_pc();

void     cpu_lock(cpu_sr * saved_psr);
void     cpu_unlock(cpu_sr * restored_psr);

cpu_sr   cpu_read_psr(void);
void     cpu_write_psr(cpu_sr * new_psr, cpu_sr * old_psr);

void     cpu_set_pil(int pil, cpu_sr * old_psr);
int      cpu_get_pil(void);


/*
 * byte swapping
 */
uint16_t cpu_bswap16(uint16_t data_16bits);
uint32_t cpu_bswap32(uint32_t data_32bits);
uint64_t cpu_bswap64(uint64_t data_64bits);

/* bit first set / bit last set / bit count */
int      cpu_bfs32(uint32_t mask);
int      cpu_bls32(uint32_t mask);
#if CPU_DATA_SIZE == 64
int      cpu_bfs_sizet(size_t size);
int      cpu_bls_sizet(size_t size);
#elif CPU_DATA_SIZE == 32
#define  cpu_bfs_sizet cpu_bfs32
#define  cpu_bls_sizet cpu_bls32
#endif


int      cpu_bones32(uint32_t mask);
uint32_t cpu_rev32(uint32_t data, int nbits);

int      cpu_init(void);
void     cpu_end(void);

void     cpu_power_down(void);

#if LIB != BIONIC_LIB
extern cpu_context_t
         cpu_global_ctx;
void     cpu_switch_context(cpu_context_t * old_ctx, cpu_context_t * new_ctx);
void     cpu_make_context(cpu_context_t * ctx, void (*task)(void* pd), void* pdata, ustack_t* ptos, size_t stk_size);
void     cpu_set_context(cpu_context_t * ctx);
#endif
void     cpu_trap_reset(void);
/*
 * Bus access
 */
int      bus_init(void);
void     bus_write32(uint32_t addr, uint32_t data);
void     bus_write16(uint32_t addr, uint16_t data);
void     bus_write8(uint32_t addr, uint8_t data);
uint32_t bus_read32(uint32_t addr);
uint16_t bus_read16(uint32_t addr);
uint8_t  bus_read8(uint32_t addr);
int      bus_write32_buf(uint32_t addr, uint32_t attribute, const uint32_t * buf, size_t nb_words);
int      bus_write16_buf(uint32_t addr, uint32_t attribute, const uint16_t * buf, size_t nb_half_words);
int      bus_write8_buf( uint32_t addr, uint32_t attribute, const uint8_t *  buf, size_t nb_bytes);
int      bus_write_buf(  uint32_t addr, uint32_t attribute, const uint8_t *  buf, size_t nb_bytes);
int      bus_read32_buf( uint32_t addr, uint32_t attribute, uint32_t * buf, size_t nb_words);
int      bus_read16_buf( uint32_t addr, uint32_t attribute, uint16_t * buf, size_t nb_half_words);
int      bus_read8_buf(  uint32_t addr, uint32_t attribute, uint8_t *  buf, size_t nb_bytes);
int      bus_read_buf(   uint32_t addr, uint32_t attribute, uint8_t *  buf, size_t nb_words);
int      bus_end();

/** HD LEON only? */
#define  CPU_POLL_PERIOD_US 10000 // 10 ms

#ifdef __cplusplus
}
#endif


#endif
