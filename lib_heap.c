#include <lib.h>
#include <stdlib.h>
#include <malloc.h>

int heap_init(void)
{
   /* TODO: check if we have someting to control the heap size */
   return 0;
}

void  heap_end(void)
{
}

void *heap_alloc(size_t size)
{
   return malloc(size);
}

void heap_free(void *ptr)
{
   free(ptr);
}

void* heap_realloc(void* ptr, size_t size)
{
   return realloc(ptr, size);
}

void* heap_align(size_t align, size_t bytes)
{
   return memalign(align, bytes);
}

void heap_dump(void)
{
   /* TODO: check if we have someting to control the heap size */
}

int heap_check()
{
   /* TODO: check if we have someting to control the heap size */
   return 0;
}


