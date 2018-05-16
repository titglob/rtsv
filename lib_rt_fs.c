#include <cpu.h>
#include <lib.h>
#include <fs.h>

int rt_file = -1;

/**
 * write a text or binary cmd into the rt_file
 */
int rt_output(const char * buffer, size_t len) 
{
   unsigned char b;
   size_t     l = len;
   int i;

   /* check that len doesn't overtake the mailbox max message size */
   if((RT_CFG_SIZE_LENGTH + len) > RT_CFG_MAX_COMMAND_LEN)
   {
      return -2;
   }

   for(i=0; i<RT_CFG_SIZE_LENGTH; i++) 
   {
      b = l & 0xFF;
      fs_write(rt_file, (char *)&b, 1);
   }

   fs_write(rt_file, (void*)buffer, len);

   return len;
}

int rt_init(const char ** env_argv)
{
   char basename[50];
   gopt_basename(env_argv[0], basename);
   string_cat(basename, ".bin");
   rt_file=fs_open(basename, FS_CREAT|FS_WRITE_ONLY|FS_TRUNC, 0666);
   return 0;
}

void rt_end()
{
   if(rt_file > 0)
      fs_close(rt_file);
}

