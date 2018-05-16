
/**
 * fs_def.h
 * abstraction of some apis from libC
 *
 */

#ifndef __FS_DEF_H__
#define __FS_DEF_H__

#include <cpu.h>
/** 
 * \addtogroup PAL
 * @{
 * \addtogroup FS
 * @{
 */
#define FS_DEF_STDIN  0
#define FS_DEF_STDOUT 1
#define FS_DEF_STDERR 2

#define FS_READ_ONLY       0x1
#define FS_WRITE_ONLY      0x2
#define FS_READ_WRITE      0x3

/**
 * \addtogroup permissions, options
 * @{
 */
#define FS_NOCTTY          0x4   //0400     =1<<10
#define FS_CREAT           0x8   //0100     =1<<8
#define FS_EXCL            0x10  //0200     =1<<9
#define FS_TRUNC           0x20  //01000    =1<<12
#define FS_APPEND          0x40  //02000    =1<<13
#define FS_NONBLOCK        0x80  //04000    =1<<14
#define FS_ASYNC           0x100 //020000   =1<<17
#define FS_DIRECT          0x200 //040000   =1<<18
#define FS_LARGEFILE       0x400 //0100000  =1<<20
#define FS_DIRECTORY       0x800 //0200000  =1<<21
#define FS_NOFOLLOW        0x1000//0400000  =1<<22
#define FS_NOATIME         0x2000//01000000 =1<<24
#define FS_CLOEXEC         0x4000//02000000 =1<<25
#define FS_NDELAY          FS_NONBLOCK
#define FS_SYNC            0x8000//04010000 =(1<<26)|1<<16
#define FS_NOCACHE         0x10000
#define FS_SWAP16          0x20000
/**@}*/

/** 
 * \addtogroup create_modes
 * @{
 */
/** modes when @ref fs_open with @ref FS_CREAT flag or for @ref fs_create*/
#define FS_MODE_RWXU 0x7   //00700
#define FS_MODE_RWU  0x6   //00600
#define FS_MODE_RUSR 0x4   //00400
#define FS_MODE_WUSR 0x2   //00200
#define FS_MODE_XUSR 0x1   //00100
#define FS_MODE_RWXG 0x70   //00070
#define FS_MODE_RWG  0x60   //00060
#define FS_MODE_RGRP 0x40   //00040
#define FS_MODE_WGRP 0x20   //00020
#define FS_MODE_XGRP 0x10   //00010
#define FS_MODE_RWXO 0x700   //00007
#define FS_MODE_RWO  0x600   //00006
#define FS_MODE_ROTH 0x400   //00004
#define FS_MODE_WOTH 0x200   //00002
#define FS_MODE_XOTH 0x100   //00001
#define FS_MODE_SUID 0x4000
#define FS_MODE_SGID 0x2000
#define FS_MODE_SVTX 0x1000
/**@}*/

/**
 * \addtogroup seek
 *  astraction for whence in @ref fs_lseek 
 * @{
 */
/** seek to start of file*/
#define FS_SEEK_SET         0
/** seek to current position in file */
#define FS_SEEK_CUR         1
/** seek to last position in file */
#define FS_SEEK_END         2
/**@}*/


/**
 * \addtogroup error_codes
 * @{
 * Defines for File System Calls
 */
#define FS_SUCCESS                    0
#define FS_ERROR                    (-1)
/**@}*/

/**
 * This structure is fill when calling @ref fs_stat
 */
typedef struct fs_stat_tag {
   int    mode;    /**< protection */
   int    size;    /**< total size, in bytes */
   int    sizeInCache;
   int    offsetInCache;
} fs_stat_t;


/**
 * Structure for @ref fs_read_dir
 */
#define FS_DIRENT_NAME_MAX_LEN (256)
typedef struct fs_dirent_tag {
   char           name[FS_DIRENT_NAME_MAX_LEN]; /**< filename */
}fs_dirent_t;

/**@} FS */
/**@} PAL*/

#endif /*__FS_DEF_H__*/
