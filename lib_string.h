

#ifndef __LIB_STRING_H__
#define __LIB_STRING_H__

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * \addtogroup PAL
 * @{
 * \addtogroup LIB
 * @{
 */

/* this is standard */
#include <cpu.h>

#include <stdarg.h>

/**
 * \addtogroup string
 * @{
 */
/**
 * string copy
 * @param[in,out] dest
 * @param[in] src to copy in dest
 * @return src
 */
char    * string_cpy(char *dest, const char *src);
/**
 * safer @ref string_cpy
 * @param[in,out] dest
 * @param[in] src to copy in dest
 * @param[in] dest max length including '\0'
 * @param[in] n size of dest
 * @return src
 */
char    * string_ncpy(char *dest, const char *src, size_t n);
/**
 * @param[in] s
 * @return length in bytes of s
 */
size_t    string_len(const char *s);

/**
 * @param[in] s : string for which we want the length
 * @param[in] max : maximum string length in case we don't find 0
 * @return length in bytes of s
 */

size_t    string_nlen(const char *s, size_t max);

/**
 * @param[in] s1
 * @param[in] s2
 * @return s1-s2 is >, < or = 0
 */
int       string_cmp(const char *s1, const char *s2);

/**
 * @param[in] s1
 * @param[in] s2
 * @param[in] n
 * @return s1-s2 is >, < or = 0
 */
int       string_ncmp(const char *s1, const char *s2, int n);

/**
 * find first occurence of needle in haystack
 * @param[in] haystack
 * @param[in] needle
 * @return ptr towards string needle found in haystack
 */
char *    string_str(const char *haystack, const char *needle);
/**
 * find first occurence of a character in a string
 * @param[in] s
 * @param[in] c
 * @return ptr towards c in s
 */

char*     string_chr(const char *s, int c);

/**
 * find last occurence of a character in a string
 * @param[in] s
 * @param[in] c
 * @return ptr towards c in s
 */
char *    string_rchr(const char *s, int c);

/**
 * find tokens in a string using a set of char delimiters
 */
char *    string_sep(char ** pstr, const char * delims);

/**
 * printf in a string
 * @param[in] str
 * @param[in] format
 * @return nbr of character printed
 */
int       string_printf(char *str, const char *format, ...);
int       string_vprintf(char *str, const char *format, va_list args);
/**
 * safer @ref string_printf
 * @param[in] str
 * @param[in] l bytes length of str
 * @param[in] format
 * @return nbr of character printed
 */
int       string_nprintf(char *str, size_t size, const char *format, ...);
int       string_vnprintf(char *str, size_t l ,const char *format, va_list args);

/**
 * concatenate strings
 * @param[in,out] dest will be appended
 * @param[in] str
 * @return new string
 */
char*     string_cat(char *dest, const char *src);

/**
 * safer @ref string_cat
 * @param[in,out] dest
 * @param[in] str
 * @param[in] n max dest length (n+1 written)
 * @return new string
 */
char*     string_ncat(char *dest, const char *src,size_t n);

/**
 * string to long it. support hexadécimal and octal format
 * @param[in] an ascii string
 * @param[out] the outputed value if success
 * @return -1 if an error occured, 0 else
 */
int string_tol(const char * ascii, long int * value);

/**@} string */
/**@} LIB */
/**@} PAL */
#ifdef __cplusplus
}
#endif


#endif /*__LIB_STD_H__*/
