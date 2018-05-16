#include <string.h>

void  mem_set(void *s, int c, size_t n)
{
   memset(s,c, n);
}

void * mem_cpy(void *dest, const void *src, size_t n)
{
   return memcpy(dest, src, n);
}

void * mem_move(void *dest, const void *src, size_t n)
{
   return memmove(dest, src, n);
}

int mem_cmp(const void *s1, const void *s2, size_t n)
{
   return memcmp(s1, s2, n);
}
