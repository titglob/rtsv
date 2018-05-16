#ifndef ARGS_H
#define ARGS_H

#include <cpu.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * \addtogroup PAL
 * @{
 * \addtogroup LIB
 * @{
 * \addtogroup gopt
 * @{
 */


/**
 * escape character.
 * annul the special meaning of the following character
 */
#define ESCAPE   '\033'

/**
 * maximum length for an option when using gopt_string, gopt_integer or gopt_bool functions
 */
#define GETOPT_CFG_MAX_OPTION_LENGTH    256

/**
 * maximum number of byte to code a long int
 */
#define GETOPT_CFG_MAX_INTEGER_LENGTH   24


/*
 * \brief convert a table argv argc of parameters into a single string (may contain ESC characters).
 *
 * max is supposed to be the maximum number of characters that we can set in \a args string
 * special case of cmd line options like > program "-a 1 -b 2" -v gives argc = 3 and argv=
 * argv[0] = 'program'
 * argv[1] = '-a 1 -b 2'
 * argv[2] = '-v'
 *
 * is then formated with gopt_string and gives args="program -a\033 1\033 -b\033 2 -v" so that when
 * calling the reverse method we retreive the smae arguments
 */
int gopt_format(int argc, char ** argv, char * args, int max);

/**
 * \brief convert a string into a table of argv[] argc strings.
 *
 * this function asserts that a table of maxArgs strings of max_argv length has been allocated by the user
 * this function do the opposite of gopt_format
 *
 * @param[in] args input string
 * @param[out] argv args strings extracted
 * @param[in] max_argc size of argv array of preallocated string of size \a max_argv
 * @param[in] max_argv max size of each string of argv array
 * @return argc
 */
int gopt_extract(char * args, char * argv, int max_argc, int max_argv);

/**
 * \brief return the next option as a string.
 *
 * Internaly, find the first non space character in \a args.
 * Then extract the string by removing all ESC characters, until we find either a non escaped space character, or the end of the string
 * the option is memset-ed to zero before starting the algorithm
 * if max limit is reached, the algorithm continue but no more characters are copied to the option string.
 * max is supposed to be the maximum number of characters that we can copy in \a option
 * if no options was found, string_len(option) egals 0
 *
 * @return NULL if no more charaters follows, or a pointer on next space character.
 */
char * gopt_next(char * args, char * option, int max);

/**
 * \brief find a string that match var_name
 * return NULL is not found 
 */
char * gopt_find(const char * var_name, char * args, int max_len);

/**
 * convert an interger ('0xf 0b1100 1234') to corresponding integer
 * @param[out] @ref value: returned value
 * @param[in] @ref ascii: formatted string containing the integer
 * @return: the ointer after the found string 
 */
char * gopt_string(char * var_val, const char * var_name, char * args, int max_len);

/**
 * convert an interger ('0xf 0b1100 1234') to corresponding integer
 * @param[out] @ref value: returned value
 * @param[in] @ref ascii: formatted string containing the integer
 * @param[in,out] last_pos we try to seach from args + last_pos. last_pos updated for next loop
 * @return: 0 if no error, -1 else 
 */
char * gopt_bool(int * var_val, const char * var_name, char * args);

/**
 * convert an interger ('0xf 0b1100 1234') to corresponding integer
 * @param[out] @ref value: returned value
 * @param[in] @ref ascii: formatted string containing the integer
 * @param[in,out] last_pos we try to seach from args + last_pos. last_pos updated for next loop
 * @return: 0 if no error, -1 else 
 */
char * gopt_integer(int * var_val, const char * var_name, char * args);
char * gopt_long   (long int * var_val, const char * var_name, char * args);

/**
 * extract from a path the file base name
 */
int    gopt_basename(const char *path, char *file);

/* @} gopt
 * @} FS
 * @} PAL */

#ifdef __cplusplus
}
#endif

#endif
