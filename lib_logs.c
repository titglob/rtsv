#include <stdarg.h>
#include <cpu.h>
#include <lib.h>
#include "lib_logs.h"
#include "log_cfg.h"
#include <time.h>
#include <stdio.h>

static char              log_tab[LOG_CFG_MAX_TAB];
static int               log_depth       = 0;
static log_handler_t     log_handler     = 0; /* old handler compat until 9.0.4 */
static log_handler_ext_t log_handler_ext = log_output_ext_def;

#if LOG_CFG_DEBUG_BUS_ENABLED == 1
static volatile
struct log_table * table          = (struct log_table *)LOG_CFG_DEBUG_BUS_TABLE; /* fast reference to log table */

static volatile
struct log_entry * entries        = (struct log_entry *)(LOG_CFG_DEBUG_BUS_TABLE + sizeof(struct log_table));
#endif

int lib_log_init()
{
   mem_set(log_tab, 0, LOG_CFG_MAX_TAB);
   log_handler_ext = log_output_ext_def;

#if LOG_CFG_DEBUG_BUS_ENABLED == 1
   /* initialization of the log table array */
   table->count  = 0;
   table->index  = 0;
   table->max    = (LOG_CFG_DEBUG_BUS_SIZE - sizeof(struct log_table)) / sizeof(struct log_entry);
   table->state  = LOG_DEF_DEBUG_BUS_RUNNING;
   mem_set((void *)entries, 0, sizeof(struct log_entry)*table->max);
#endif

   return 0;
}

int lib_log_end()
{
   return 0;
}

int lib_set_log_handler(log_handler_ext_t new_handler, log_handler_ext_t * old_handler)
{
   if(old_handler)
      *old_handler = log_handler_ext;

   log_handler_ext = new_handler;
   return 0;
}

void log_tab_inc()
{
  if (log_depth < (LOG_CFG_MAX_TAB - 1))
    log_depth++;

  log_tab[log_depth] = '\0';
  log_tab[log_depth-1] = '\t';
}

void log_tab_dec()
{
  if (log_depth > 0)
    log_depth--;

  log_tab[log_depth] = '\0';
}

void log_print(const char * module, int options, int severity, const char * color, int output, const char * fmt, ...)
{
   const char * sev;
   char text[LOG_CFG_MAX_LOG_LENGTH + 1];
   char * p = &text[0];
   int l = 0;

#if LOG_CFG_COLORED_TRACES == 0
   (void)color;
#endif

   mem_set(text, 0, LOG_CFG_MAX_LOG_LENGTH + 1);

   /* header */
   if((options & LOG_HAVE_PREV) == 0)
   {
      /* module */
      if(module) {
         l += string_nprintf(p + l, LOG_CFG_MAX_LOG_LENGTH - l, "[" LOG_CFG_MAX_MOD_FMT "] ", module);
      }

      /* severity */
      if(severity == DEBUG_ERROR)
         sev = "ERROR : ";
      else if(severity == DEBUG_INFO)
         sev = "INFO : ";
      else if(severity == DEBUG_VERB)
         sev = "VERB : ";
      else
         sev = "";
      l += string_nprintf(p + l, LOG_CFG_MAX_LOG_LENGTH - l, sev);

      /* time */
      if(options & LOG_TIME)
      {
			struct tm * ts = localtime(NULL);
         l += string_nprintf(p + l, LOG_CFG_MAX_LOG_LENGTH - l, "#%2dh %2dm %2ds# ", ts->tm_hour, ts->tm_min, ts->tm_sec);
      }

      /* tabulation */
      l += string_nprintf(p + l, LOG_CFG_MAX_LOG_LENGTH - l, log_tab);

#if LOG_CFG_COLORED_TRACES == 1
      /* color */
      if(color)
         l += string_nprintf(p + l, LOG_CFG_MAX_LOG_LENGTH - l, color);
#endif
   }

   /* formated traces */
   va_list args;
   va_start (args, fmt);
   l += string_vnprintf(p + l, LOG_CFG_MAX_LOG_LENGTH - l, fmt, args);
   va_end (args);

   /* foot */
   if((options & LOG_HAVE_NEXT) == 0)
   {
#if LOG_CFG_COLORED_TRACES == 1
      if(color)
         l += string_nprintf(p + l, LOG_CFG_MAX_LOG_LENGTH - l, COLOR_DEFAULT);
#endif
   }

   /* old handler compat until 9.0.4. Its definition overwritte the extended one */
   if(log_handler)
      (*log_handler)(output, text);
   else if(log_handler_ext)
      (*log_handler_ext)(severity, output, text);
}

/**
 * low level log handler function
 */
void log_output_ext_def(int severity, int output, char * text)
{
   if (output == LOG_DEF_PRINT || output == LOG_DEF_MBX)
   {
      FILE * out_fd;
      cpu_sr psr;
      if(severity < DEBUG_INFO)
         out_fd = stderr;
      else
         out_fd = stdout;

      fprintf(out_fd, "%s", text);
   }
}

/**
 * old handler, kept for compat until 9.0.4 version
 */
void log_output_def(int output, char * text)
{
   log_output_ext_def(DEBUG_INFO, output, text);
}



#if LOG_CFG_DEBUG_BUS_ENABLED == 1

/*
 * low level read clock time function, bypassing log_access to avoid recursion
 */
static uint32_t log_time(void)
{
#if CPU == LEON
   return cpu_read32(0x80007004);
#else
   return clock_read();
#endif
}

/**
 * store every R/W access into a global circular buffer array
 */
void log_access(uint32_t addr, uint32_t data, uint32_t attr)
{
   volatile struct log_entry * le;

   // host write protection
   if(table->state != LOG_DEF_DEBUG_BUS_RUNNING)
      return;

   if(attr & LOG_DEF_ATTR_READ_DATA)
   {
      int index = (table->index + table->max - 1) % table->max;
      le = &entries[index];
      le->attr |= LOG_DEF_ATTR_READ_DATA;
      le->data = data;
      le->time = log_time();
   }
   else
   {
      le = &entries[table->index];
      le->addr = addr;
      le->attr = attr;
      le->data = data;
      le->time = log_time();
      table->index = (table->index + 1) % table->max;
      table->count++;
   }
}

#endif

/**
 * display log entry information
 * \param n a position
 */
void log_print_entry(struct log_entry * le, int n)
{
   int      ts;    // transfer size
   char     rw, val;

   if(le->attr & LOG_DEF_ATTR_BIT_8)
      ts = 8;
   else if(le->attr & LOG_DEF_ATTR_BIT_16)
      ts = 16;
   else if(le->attr & LOG_DEF_ATTR_BIT_32)
      ts = 32;
   else
      ts = 0;

   if((le->attr & LOG_DEF_ATTR_READ) && !(le->attr & LOG_DEF_ATTR_READ_DATA))
      val = 'X';
   else
      val = ' ';
   
   if(le->attr & LOG_DEF_ATTR_READ)
      rw = 'R';
   else if(le->attr & LOG_DEF_ATTR_WRITE)
      rw = 'W';
   else
      rw = '?';

   fprintf(stdout, "\t%3d : [%10d] %c%-2d addr=%08x data=%08x%c\n",
      n,
      le->time,
      rw,
      ts,
      le->addr,
      le->data,
      val);
}



