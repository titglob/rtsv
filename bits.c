
#include <signal.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <endian.h>
#include <pthread.h>
#include "cpu.h"
/*
 * swap functions
 */
uint16_t cpu_bswap16(uint16_t host_16bits)            { return htobe16(host_16bits); }
uint32_t cpu_bswap32(uint32_t host_32bits)            { return htobe32(host_32bits); }
uint64_t cpu_bswap64(uint64_t host_64bits)            { return htobe64(host_64bits); }

/*
 * bit first set and bit last set functions
 */
int cpu_bfs32(uint32_t mask)
{
   return __builtin_ffs(mask) - 1;
}

int cpu_bls32(uint32_t mask)
{
   const int bit = mask ? 32 - __builtin_clz(mask) : 0;
   return bit - 1;
}

#if CPU_DATA_SIZE == 64
int cpu_bls_sizet(size_t size)
{
   uint32_t high = (uint32_t)(size >> 32);
   int bits = 0;
   if (high)
   {
      bits = 32 + cpu_bls32(high);
   }
   else
   {
      bits = cpu_bls32((uint32_t)size & 0xffffffff);

   }
   return bits;
}
#endif


/*
 * Return the number of bits that are set to one in Mask
 */
int cpu_bones32(uint32_t mask)
{
   if(mask)
   {
      int i = 0;
      while(mask)
      {
         if(mask & 1) i++;
         mask = mask >> 1;
      }
      return i;
   }
   else
   {
      return 0;
   }
}


/*
 * reverse the bits of a word
 */
uint32_t cpu_rev32(uint32_t data, int nbits)
{
   unsigned long  reversed = 0x00000000;
   unsigned char  bit;

   /*
    * reverse the data about the center bit.
    */
   for (bit = 0; bit < nbits; ++bit)
   {
      /*
       * if the lsb bit is set, set the reflection of it.
       */
      if (data & 0x01)
      {
         reversed |= (1 << ((nbits - 1) - bit));
      }

      data = (data >> 1);
   }
   return (reversed);
}

