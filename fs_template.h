#ifndef FS_TEMPLATE_H
#define FS_TEMPLATE_H

#include <cpu.h>

#include <fs_def.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \addtogroup PAL
 * @{
 * \addtogroup FS
 * @{
 *
 * do not include this file directly in user code, it is used as a template for lib platform implementations.
 * This file require the definition of some types to be used
 * - fs_dirent_t
 * - fs_dirp_t
 */

/**
 * @brief Initializes the File System functions
 * @return 0 if not error occured
 */

int           fs_init(void);

/**
 * @brief Clean the File System functions
 * @return 0 if not error occured
 */

int           fs_end(void);

/**
 * @brief Creates a file specified by path
 * @param[in] path string to indicate relative or full path of file to create
 * @param[in] mode @ref modes
 * @return -1 or a file descriptor >= 0
 */
int           fs_create(const char *path, int mode);

/**
 * @brief Open a file for reading/writing. Returns file descriptor
 * @param[in] path string to indicate relative or full path of file to use
 * @param[in] access @ref access
 * @param[in] mode @ref modes
 * @return -1 or a file descriptor >= 0
 */
int           fs_open(const char *path,  int access,  int mode);

/**
 * @brief Close an open file.
 * @pre @ref fs_create or @ref fs_open
 * @param[in] filedes -1 or a file descriptor >= 0
 * @return error_codes
 */
int           fs_close(int filedes);

/**
 * @brief Reads nbytes bytes from file into buffer
 * @param[in] filedes -1 or a file descriptor >= 0
 * @param[in,out] buffer data read
 * @param[in] nbytes buffer size in bytes
 * @return -1 or the number of bytes read
 */
int           fs_read(int filedes, void *buffer, int nbytes);

/**
 * @brief Write nybytes bytes of buffer into the file
 * @param[in] filedes -1 or a file descriptor >= 0
 * @param[in] buffer data to write
 * @param[in] nbytes buffer size in bytes
 * @return -1 or the number of bytes written
 */
int           fs_write(int filedes, void *buffer, int nbytes);

/**
 * @brief Changes the permissions of a file
 * @param[in] path string to indicate relative or full path of file to use
 * @param[in] mode @ref modes
 * @return @ref error_codes
 */
int           fs_chmod(const char *path, int mode);

/**
 * @brief Returns file status information in filestats
 * @param[in] path string to indicate relative or full path of file to use
 * @param[in] filestats @ref fs_stat_t
 * @return @ref error_codes
 */
int           fs_stat(const char *path, fs_stat_t *filestats);

/**
 * @brief Seeks to the specified position of an open file
 * @param[in] filedes -1 or a file descriptor >= 0
 * @param[in] offset number of bytes
 * @param[in] whence @ref whence
 * @return @ref error_codes
 */
int           fs_lseek(int filedes, int32_t offset, int whence);

/**
 * @brief Removes a file from the file system
 * @param[in] path string to indicate relative or full path of file to use
 * @return @ref error_codes
 */
int           fs_remove_file(const char *path);

/**
 * @brief Renames a file in the file system
 * @param[in] old_file string to indicate relative or full path of file to rename
 * @param[in] new_file string to indicate relative or full path of file to create
 * @return @ref error_codes
 */
int           fs_rename_file(const char *old_file, const char *new_file);

/**
 * @brief copies a single file from src to dest
 * @param[in] src string to indicate relative or full path of file to copy
 * @param[in] dest string to indicate relative or full path of file to create with src content
 * @return @ref error_codes
 */
int           fs_copy_file(const char *src, const char *dest);

/**
 * @brief moves a single file from src to dest
 * @param[in] src string to indicate relative or full path of file to move
 * @param[in] dest string to indicate relative or full path of file to create with src content
 * @return @ref error_codes
 */
int           fs_move_file(const char *src, const char *dest);


/**
 * @brief Makes a new directory
 * @param[in] path string to indicate relative or full path of directory to create
 * @param[in] mode @ref modes
 * @return @ref error_codes
 */
int           fs_make_dir(const char *path, int mode);

/**
 * @brief Opens a directory for searching
 * @param[in] path string to indicate relative or full path of directory to open
 * @return @ref fs_dirp_t
 */
fs_dirp_t     fs_open_dir(const char *path);

/**
 * @brief Closes an open directory
 * @pre @ref fs_open_dir
 * @param[in] directory indentified by the struct returned by @ref fs_open_dir
 * @return @ref error_codes
 */
int           fs_close_dir(fs_dirp_t directory);

/**
 * @brief Reads the next object in the directory
 * @pre @ref fs_open_dir
 * @param[in] directory indentified by the struct returned by @ref fs_open_dir
 * @return @ref fs_dirent_t
 */
fs_dirent_t * fs_read_dir(fs_dirp_t directory);

/**
 * @brief Removes an empty directory from the file system.
 * @param[in] path string to indicate relative or full path of directory
 * @return @ref error_codes
 */
int           fs_remove_dir(const char *path);

/**
 * @brief get the current working directory from the file system.
 * @return current working directory
 */
char *        fs_get_cwd(void);

/**
 * @brief set the current working directory.
 * @ref error_codes
 */
int           fs_set_cwd(const char * path);

/**
 * @brief check if it is a relative path or not
 * @ref error_codes
 */
int           fs_is_relative_path(const char * path);

/**
 * @brief set the working directory to home directory
 * @ref error_codes
 */
int           fs_set_cwd_to_home_directory(void);

/**
 * Return the string associated to an environment variable
 */
char        * fs_get_env(const char * var);

/* @} FS
 * @} PAL */
#ifdef __cplusplus
}
#endif

#endif /*FS_TEMPLATE_H*/
