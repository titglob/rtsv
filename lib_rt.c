#include <cpu.h>
#include <lib.h>

#define _MAX_(_x, _y)          \
   ({                        \
      typeof(_x) __x = (_x); \
      typeof(_y) __y = (_y); \
      __x > __y ? __x : __y; \
    })

#if RT_OBJECT_SIZE == 4
#define lib_htobe lib_htobe32
#else
#define lib_htobe lib_htobe64
#endif

/**
 * Network time management
 */
static rt_time_t rt_offset = 0;          /// ajustable offset to synchronize network time
static rt_time_t rt_transmit_time = 100; /// 1 ms?
static rt_time_t rt_last_time = 0;

#if RT_CFG_RTCLI_EN == 1
/**
 * general file format for traces
 * 1: text, 0:binary
 */
static int rt_format = 0;

/**
 * a temporar buffer to store the command
 */
static char  rt_cmd_buffer[RT_CFG_MAX_COMMAND_LEN];
#endif



/**
 * command key names
 */
static const char * rt_key[RT_DEF_CMD_MAX] =
{
   [RT_DEF_CMD_DECLTASK]  = "decl_task",
   [RT_DEF_CMD_DECLMUTEX] = "decl_mutex",
   [RT_DEF_CMD_DECLOBJ]   = "decl_object",
   [RT_DEF_CMD_SENDMSG]   = "send_msg",
   [RT_DEF_CMD_CALL]      = "call",
   [RT_DEF_CMD_RECVMSG]   = "recv_msg",
   [RT_DEF_CMD_SWITCH]    = "switch",
   [RT_DEF_CMD_RETURN]    = "return",
   [RT_DEF_CMD_COMMENT]   = "comment",
   [RT_DEF_CMD_ACTION]    = "action",
   [RT_DEF_CMD_SETTIMER]  = "set_timer",
   [RT_DEF_CMD_TIMEOUT]   = "timeout",
   [RT_DEF_CMD_STOPTIMER] = "stop_timer",
   [RT_DEF_CMD_CREATTASK] = "create_task",
   [RT_DEF_CMD_CREATMUTEX]= "create_mutex",
   [RT_DEF_CMD_CREATOBJ]  = "create_object",
   [RT_DEF_CMD_TAKE]      = "take",
   [RT_DEF_CMD_GIVE]      = "give",
   [RT_DEF_CMD_DELMUTEX]  = "del_mutex",
   [RT_DEF_CMD_DELTASK]   = "del_task",
   [RT_DEF_CMD_DELOBJ]    = "del_object",
   [RT_DEF_CMD_DECLBOOL]  = "decl_bool",
   [RT_DEF_CMD_DECLWIRE]  = "decl_wire",
   [RT_DEF_CMD_DECLINT]   = "decl_int",
   [RT_DEF_CMD_DECLREAL]  = "decl_real",
   [RT_DEF_CMD_DECLSTRING]= "decl_string",
   [RT_DEF_CMD_SETINT]    = "set_int",
   [RT_DEF_CMD_SETREAL]   = "set_real",
   [RT_DEF_CMD_SETSTRING] = "set_string",
   [RT_DEF_CMD_SETSTATE]  = "set_state",
   [RT_DEF_CMD_CREATGRP]  = "create_group",
   [RT_DEF_CMD_DELGRP]    = "del_group",
   [RT_DEF_CMD_ACQUIRE]   = "acquire",
   [RT_DEF_CMD_DECLEVENT] = "decl_event",
   [RT_DEF_CMD_DECLTIME]  = "decl_time",
   [RT_DEF_CMD_DECLPARAM] = "decl_param",
   [RT_DEF_CMD_DECLREG]   = "decl_reg",
   [RT_DEF_CMD_SETEVENT]  = "set_event",
   [RT_DEF_CMD_SETPARAM]  = "set_param",
   [RT_DEF_CMD_SETREG]    = "set_reg",
   [RT_DEF_CMD_SETTIME]   = "set_time",
   [RT_DEF_CMD_SETBOOL]   = "set_bool",
   [RT_DEF_CMD_SETWIRE]   = "set_wire",
   [RT_DEF_CMD_STARTDUMP] = "start_dump",
   [RT_DEF_CMD_STOPDUMP]  = "stop_dump",
   [RT_DEF_CMD_DELVAR]    = "del_var",
   [RT_DEF_CMD_SETGLOBAL] = "set_global",
   [RT_DEF_CMD_READY]     = "ready",
   [RT_DEF_CMD_RUN]       = "run",
   [RT_DEF_CMD_PREEMPT]   = "preempt",
   [RT_DEF_CMD_IDLE]      = "idle",
   [RT_DEF_CMD_WAIT]      = "wait"
};

const char * rt_cmd_name(rt_cmd_t cmd)
{
   if((cmd < 0) || (cmd >= RT_DEF_CMD_MAX))
      return NULL;

   return rt_key[cmd];
}

uint32_t clock_read(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (uint32_t)ts.tv_nsec;
}

/*-----------------------------------------------------------------------------------------
 * Time management
 *---------------------------------------------------------------------------------------*/

rt_time_t rt_time(void)
{
   rt_time_t time = clock_read() + rt_offset;
   if(rt_last_time && (rt_last_time == time))
   {
      rt_offset += 1;
      time += 1;
   }
   rt_last_time = time;
   return time;
}

/**
 * Return last register time
 */
static rt_time_t rt_last(void)
{
   return rt_last_time;
}

rt_time_t rt_sync(rt_time_t external_time)
{
   rt_time_t local_time = clock_read();
   rt_time_t adjusted_time = _MAX_(external_time + rt_transmit_time, local_time + rt_offset);
   rt_offset = adjusted_time - local_time;  // reajust the rt_offset
   return adjusted_time;
}

/*-----------------------------------------------------------------------------------------
 * Message formatting
 *---------------------------------------------------------------------------------------*/

#if RT_CFG_RTCLI_EN == 1

/**
 * Pack rt_msg arguments to a buffer.
 * Return the packed length, or < 0 if an error occured
 */
static int rt_msg_to_buf(char * buf, int bufmax, rt_cmd_t cmd, rt_time_t time,
                         object_id_t grp, object_id_t id1, object_id_t id2,
                         const char * text)
{
   int sz = RT_OBJECT_SIZE;
   const int min = 1 + sizeof(rt_time_t) + 3 * sz;
   int tlen = 0;
   int n = 0;

   tlen = string_len(text) + 1;

   if(tlen + min > bufmax)
      return -1;

   *(buf) = cmd;

   // add a flag for 64bit object_id
   if(sz == 8)
      *(buf) |= (1 << 7);

   n += 1;
   time = lib_htobe32(time);
   mem_cpy(buf + n, &time, sizeof(rt_time_t));
   n += sizeof(rt_time_t);
   grp = lib_htobe(grp);
   mem_cpy(buf + n, &grp, sz);
   n += sz;
   id1 = lib_htobe(id1);
   mem_cpy(buf + n, &id1, sz);
   n += sz;
   id2 = lib_htobe(id2);
   mem_cpy(buf + n, &id2, sz);
   n += sz;

   string_cpy(buf + n, text);
   n += tlen;

   return n;
}

/**
 * Write a null terminated string corresponding to the rt_command, provided rt_msg arguments
 * Return the number of bytes written, including the null termniated string, or < 0 if an error occured
 */
static int rt_msg_to_string(char * string, int strmax, rt_cmd_t cmd,
                            rt_time_t time, object_id_t grp, object_id_t id1,
                            object_id_t id2, const char * text)
{
   int tlen = 0;
   int n = 0;

   if((cmd >= RT_DEF_CMD_MAX) || (cmd < 0))
      return -1;

   return string_nprintf(string, strmax, "%s @%d #0x%x 0x%x 0x%x %s\n", rt_key[cmd], time, grp, id1, id2, text);
}

void rt_log(rt_time_t time, rt_cmd_t cmd, object_id_t grp, object_id_t id1, object_id_t id2, const char * name)
{
   cpu_sr psr;
   int len;

   if(time == RT_CUR)
      time=rt_time();
   else if(time == RT_LAST)
      time=rt_last();
   else if(time == RT_ORIG)
      time=0;

   if(rt_format)
      len = rt_msg_to_string(rt_cmd_buffer, RT_CFG_MAX_COMMAND_LEN, cmd, time, grp, id1, id2, name);
   else
      len = rt_msg_to_buf(rt_cmd_buffer, RT_CFG_MAX_COMMAND_LEN, cmd, time, grp, id1, id2, name);

   rt_output(rt_cmd_buffer, len);
}


#endif

#if RT_CFG_RTSV_EN == 1
int rt_msg_from_buf(char * buf, int len, rt_cmd_t * cmd, rt_time_t * time, object_id_t * grp, object_id_t * id1, object_id_t * id2, char * text)
{
   int flag_64bit;
   int sz;

   *cmd = (rt_cmd_t)((*buf) & 0x7F);
   if((*cmd < 0) || (*cmd >= RT_DEF_CMD_MAX))
      return -1;
   flag_64bit = ((*buf) >> 7) & 1;

   buf += 1;

   sz = flag_64bit ? 8 : 4;

   const int min = 1 + sizeof(rt_time_t) + 3 * sz;

   if(len < min)
      return -1;

   mem_cpy(time, buf, sizeof(rt_time_t));
   *time = lib_htobe32(*time);
   buf += sizeof(rt_time_t);

   mem_cpy(grp, buf, sz);
   *grp = flag_64bit ? lib_htobe64(*grp) : lib_htobe32(*grp);
   buf += sz;

   mem_cpy(id1, buf, sz);
   *id1 = flag_64bit ? lib_htobe64(*id1) : lib_htobe32(*id1);
   buf += sz;

   mem_cpy(id2, buf, sz);
   *id2 = flag_64bit ? lib_htobe64(*id2) : lib_htobe32(*id2);
   buf += sz;

   len -= min;

   string_ncpy(text, buf, len);

   return 0;
}

/**
 * Parse a command and fill the rt_message structure
 * General syntax:
 *      cmd @time [#grp] [id1] [id2] text....
 * st =  0    1      2     3     4    5
 */
int rt_msg_from_string(char * string, rt_cmd_t * cmd, rt_time_t * time, object_id_t * grp, object_id_t * id1, object_id_t * id2, char * text)
{
   char * tok;
   char * str = string;
   const char * delim = " \t";
   long int v;
   int i;
   int st = 0;
   int parse = 1;

   *id1 = 0;
   *id2 = 0;
   *grp = 0;
   *text = '\0';
   *cmd = RT_DEF_CMD_MAX;

   do 
   {
      tok = string_sep(&str, delim);

      if(tok == NULL)
         break;

      // discard empty strings
      if(string_len(tok) == 0)
         continue;

      // printf("st %d tok = '%s' str='%s'\n", st, tok, str);
      if(st == 0)
      {
         for (i = 0; i < RT_DEF_CMD_MAX; i++)
         {
            if (string_cmp(tok, rt_key[i]) == 0)
            {
               *cmd = (rt_cmd_t)i;
               break;
            }
         }
         if (i == RT_DEF_CMD_MAX)
            return -1;
         st = 1;
      }
      else if(st <= 4)
      {
         if(st == 1)
         {
            if (tok[0] != '@') // time is mandatory
               return -2;
            tok++;
         }
         else if(st == 2)
         {
            if (tok[0] != '#') // group declaration is optional
               st++;
            else
               tok++;
         }

         if(string_tol(tok, &v) != 0)
         {
            if(st == 1)
               return -1;

            st = 5;
            string_cat(text, tok);
            continue;
         }

         if(st == 1)
            *time = v;
         else if(st == 2)
            *grp = v;
         else if(st == 3)
            *id1 = v;
         else
            *id2 = v;

         st++;
      }
      else if(st == 5)
      {
         if(*text != '\0')
            string_cat(text, " ");
         string_cat(text, tok);
      }
   }
   while(str);

   // discard empty lines
   if(*cmd == RT_DEF_CMD_MAX)
      return -1;

   return 0;
}

#endif

