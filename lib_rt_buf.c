#include <lib.h>
#include <cpu.h>

/**
 * Size of the embedded trace buffer
 */

/* mainly needed for embedded to place the data at a precise memory address */
#define __TRACEBUF __attribute__((section(".tracebuf")))

struct rt_trace_buffer rt_trace_buf        __TRACEBUF;

/**
 * low level write to the ring buffer
 */
static uint32_t _write(uint32_t dest, uint32_t from, int len)
{
   if (dest + len >= rt_trace_buf.end)
   {
      mem_cpy((void*)dest, (void*)from, (rt_trace_buf.end - dest));
      len -= rt_trace_buf.end - dest;
      from += (rt_trace_buf.end - dest);
      dest = rt_trace_buf.start;
   }

   if (len > 0)
   {
      mem_cpy((void*)dest, (void*)from, len);
      dest += len;
   }
   return dest;
}

/**
 * count the number of free bytes between read and write
 */
static size_t _free_bytes(uint32_t read, uint32_t write)
{
   size_t space;
   if (read > write)
      space = read - write;
   else
      space = rt_trace_buf.size - (write - read);

   return space;
}

int rt_init(void)
{
   extern uint32_t _tracemem_start;
   extern uint32_t _tracemem_size;
   extern uint32_t _tracemem_end;

   rt_trace_buf.start   = (uint32_t)&_tracemem_start;
   rt_trace_buf.wrptr   = (uint32_t)&_tracemem_start;
   rt_trace_buf.rdptr   = (uint32_t)&_tracemem_start;
   rt_trace_buf.end     = (uint32_t)&_tracemem_end;
   rt_trace_buf.size    = (uint32_t)&_tracemem_size;

   rt_trace_buf.errov   = 0;
   rt_trace_buf.errsize = 0;

   return 0;
}


/**
 * write a text or binary cmd into a circular buffer.
 */
int rt_output(const char * buf, size_t len) 
{
   unsigned char b;
   size_t       l = len;
   uint32_t     rdptr;
   uint32_t     wrptr;
   uint32_t     buffer = (uint32_t)buf;
   int i;

   /* check that len doesn't overtake the mailbox size */
   if((RT_CFG_SIZE_LENGTH + len) >= rt_trace_buf.size)
   {
      rt_trace_buf.errsize++;
      return -1;
   }

   /* check that len doesn't overtake the mailbox max message size */
   if((RT_CFG_SIZE_LENGTH + len) > RT_CFG_MAX_COMMAND_LEN)
   {
      rt_trace_buf.errsize++;
      return -2;
   }
   wrptr = rt_trace_buf.wrptr;
   rdptr = bus_read32((uint32_t)&rt_trace_buf.rdptr);

   /* we must have at least info bytes */
   if(_free_bytes(rdptr, wrptr) <= (RT_CFG_SIZE_LENGTH + len))
   {
      rt_trace_buf.errov++;
      return 0;
   }

   for(i=0; i<RT_CFG_SIZE_LENGTH; i++) 
   {
      b = l & 0xFF;
      wrptr = _write(wrptr, (uint32_t)&b, 1);
      l >>= 8;
   }

   /* copy the user message then */
   wrptr = _write(wrptr, buffer, len);

   /* write pointer update at the end */
   rt_trace_buf.wrptr = wrptr;

   return len;
}

void rt_end()
{
}
