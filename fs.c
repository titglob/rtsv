#define LOG_MODULE _FS_
#include <log_cfg.h>

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <lib_string.h>
#include <pwd.h>
#include "fs.h"

static mode_t fs_conv_mode(int mode)
{
   mode_t m = 0;
   if((FS_MODE_RWXU & mode) == FS_MODE_RWXU)
   {
      m |= S_IRWXU;
   }
   else
   {
      if(FS_MODE_RUSR & mode)
         m |= S_IRUSR;
      if(FS_MODE_WUSR & mode)
         m |= S_IWUSR;
      if(FS_MODE_XUSR & mode)
         m |= S_IXUSR;
   }
   if((FS_MODE_RWXG & mode) == FS_MODE_RWXG)
   {
      m |= S_IRWXG;
   }
   else
   {
      if(FS_MODE_RGRP & mode)
         m |= S_IRGRP;
      if(FS_MODE_WGRP & mode)
         m |= S_IWGRP;
      if(FS_MODE_XGRP & mode)
         m |= S_IXGRP;
   }
   if((FS_MODE_RWXO & mode)  == FS_MODE_RWXO)
   {
      m |= S_IRWXO;
   }
   else
   {
      if(FS_MODE_ROTH & mode)
         m |= S_IROTH;
      if(FS_MODE_WOTH & mode)
         m |= S_IWOTH;
      if(FS_MODE_XOTH & mode)
         m |= S_IXOTH;
   }
   if(FS_MODE_SUID & mode)
      m |= S_ISUID;
   if(FS_MODE_SGID & mode)
      m |= S_ISGID;
   if(FS_MODE_SVTX & mode)
      m |= S_ISVTX;
   return m;
}

int fs_init(void)
{
   return 0;
}

int fs_end(void)
{
   return 0;
}

int fs_create(const char *path, int mode)
{
   return /* fd or -1 */creat(path,fs_conv_mode(mode));
}

/** mode when O_CREATE in the access flags */
int fs_open(const char * path, int access, int mode)
{
   int flags = 0;
   int fd;

   INFO("try to open %s\n", path);

   // open flags
   if((FS_READ_WRITE & access) == FS_READ_WRITE)
      flags |= O_RDWR;
   else if((FS_READ_ONLY & access) == FS_READ_ONLY)
      flags |= O_RDONLY;
   else if((FS_WRITE_ONLY & access) == FS_WRITE_ONLY)
      flags |= O_WRONLY;

   //creation/status
   if(FS_CREAT & access)
      flags |= O_CREAT;
   if(FS_EXCL & access)
      flags |= O_EXCL;
   if(FS_TRUNC & access)
      flags |=  O_TRUNC;
   if(FS_APPEND & access)
      flags |= O_APPEND;
   if(FS_NONBLOCK & access)
      flags |= O_NONBLOCK;
   if(FS_ASYNC & access)
      flags |= O_ASYNC;
   if(FS_DIRECTORY & access)
      flags |= O_DIRECTORY;
   if(FS_NOFOLLOW & access)
      flags |= O_NOFOLLOW;
   if(FS_CLOEXEC & access)
      flags |= O_CLOEXEC;
   if(FS_NDELAY & access)
      flags |= O_NDELAY;
   if(FS_SYNC & access)
      flags |= O_SYNC;

   fd = open(path, flags, fs_conv_mode(mode));

   INFO("fd <= %d\n", fd);
   return fd;
}

int fs_close(int filedes)
{
   INFO("fs_close\n", filedes);
   return /*0 or -1 */close(filedes);
}

int fs_read(int filedes, void *buffer, int nbytes)
{
   INFO("fs_read fd=%d len=%d\n", filedes, nbytes);
   return /*x>=0 or -1*/read(filedes,buffer,(size_t)nbytes);
}

int fs_write(int filedes, void *buffer, int nbytes)
{
   INFO("fs_write fd=%d len=%d\n", filedes, nbytes);
   return /*x>=0 or -1*/write(filedes,buffer,nbytes);
}

int fs_chmod (const char *path, int mode)
{
   return /*0 or -1 */chmod(path,fs_conv_mode(mode));
}

int fs_stat(const char * path, fs_stat_t  *filestats)
{
   int ret;
   struct stat s;

   ret = /*0 or -1 */stat(path, &s);

   filestats->mode = 0;

   if(s.st_mode == O_RDONLY)
   {
      filestats->mode |= FS_READ_ONLY;
   }
   if(s.st_mode == O_WRONLY)
   {
      filestats->mode |= FS_WRITE_ONLY;
   }
   if(s.st_mode == 0)
   {
      filestats->mode |= FS_READ_WRITE;
   }

   filestats->size = s.st_size;
   filestats->sizeInCache = 0;
   filestats->offsetInCache = 0;

   return ret; // TODO
}

static int fs_convert_whence(int whence)
{
   int out = 0;
   switch(whence)
   {
   case FS_SEEK_SET: out = SEEK_SET; break;
   case FS_SEEK_CUR: out = SEEK_CUR; break;
   case FS_SEEK_END: out = SEEK_END; break;
   }
   return out;

}
int fs_lseek(int filedes, int32_t offset, int whence)
{
   return /*off_t or -1 */lseek(filedes,offset,fs_convert_whence(whence));
}

int fs_remove_file(const char *path)
{
   return /*0 or -1 */remove(path);
}

int fs_rename_file(const char *old_file, const char *new_file)
{
   return /*0 or -1 */rename(old_file, new_file);
}

int fs_copy_file(const char *src, const char *dest)
{
   (void) src;
   (void) dest;
   return -1;
}
int fs_move_file(const char *src, const char *dest)
{
   (void) src;
   (void) dest;
   return -1;
}

/******************************************************************************
** Directory API
******************************************************************************/
static fs_dirent_t  fs_dir;

int fs_make_dir(const char *path, int mode)
{
   return mkdir(path,fs_conv_mode(mode));
}

fs_dirp_t fs_open_dir(const char *path)
{
   return (fs_dirp_t)opendir(path);
}

int fs_close_dir(fs_dirp_t directory)
{
   return closedir((DIR*)directory);
}

fs_dirent_t * fs_read_dir(fs_dirp_t directory)
{
   struct dirent * dir = 0;
   dir = readdir((DIR*)directory);
   string_ncpy(fs_dir.name, dir->d_name, FS_DIRENT_NAME_MAX_LEN);
   fs_dir.name[FS_DIRENT_NAME_MAX_LEN-1]='\0';
   return &fs_dir;
}

int fs_remove_dir(const char *path)
{
   return rmdir(path);
}

char * fs_get_cwd(void)
{
   return getcwd(NULL, 0);
}

int fs_set_cwd(const char * path)
{
   INFO("set current directory to %s\n", path);

   return chdir(path);
}

char * fs_get_env(const char * var)
{
   return getenv(var);
}

int fs_is_relative_path(const char * path)
{
   if(string_ncmp(path, "/", 1) == 0 || string_ncmp(path, "~", 1) == 0)
      return 0;
   else
      return 1;
}
