
#include <lib_string.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

char * string_cpy(char *dest, const char *src)
{
   /* coverity[secure_coding] */
   return strcpy(dest,src);
}


char * string_ncpy(char *dest, const char *src, size_t n)
{
   return strncpy(dest,src,n);
}


size_t string_len(const char *s) 
{ 
   return strlen(s);
}

size_t string_nlen(const char *s, size_t m) 
{ 
   return strnlen(s, m);
}



int string_cmp(const char *s1, const char *s2)
{
   return strcmp(s1,s2);
}


int string_ncmp(const char *s1, const char *s2, int n)
{
   return strncmp(s1,s2,n);
}


char * string_str(const char *haystack, const char *needle)
{
   return strstr(haystack,needle);
}


char * string_chr(const char *s, int c)
{
   return strchr(s, c);
}

char * string_rchr(const char *s, int c)
{
   return strrchr(s, c);
}

char * string_sep(char ** pstr, const char * delims)
{
   return strsep(pstr, delims);
}

int string_vprintf(char *str, const char *format, va_list arglist)
{
   return vsprintf(str,format,arglist);
}

int string_vnprintf(char *str, size_t size, const char *format, va_list arglist)
{
   return vsnprintf(str,size,format,arglist);
}

int string_printf(char *str, const char *format, ...)
{
   int ret;
   va_list arglist;

   va_start( arglist, format );
   ret=vsprintf(str,format,arglist);
   va_end( arglist );
   return ret;
}
 
int string_nprintf(char *str, size_t size, const char *format, ...)
{
   int ret;
   va_list arglist;

   va_start( arglist, format );
   ret=vsnprintf(str,size,format,arglist);
   va_end( arglist );
   return ret;
}

char* string_cat(char *dest, const char *src)
{
   /* coverity[secure_coding] */
   return strcat(dest,src);
}

char *string_ncat(char *dest, const char *src,size_t n)
{
   return strncat(dest,src,n);
}


int string_tol(const char * ascii, long int * value)
{
   long int v;
   char * endptr;

   v = strtol(ascii, &endptr, 0);

   *value = v;

   // case where no digits are found
   if(endptr == ascii)
      return -1;

   return 0;
}

