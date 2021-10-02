

#include "rtsv.h"

/**
 * configuration of logs for this file
 */
#define _DEF__LOG_NAME     "RTSV"
#define _DEF__LOG_LEVEL    DEBUG_VERB
#define _DEF__LOG_COLOR    COLOR_LIGHT_BLUE
#define _DEF__LOG_OUTPUT   LOG_DEF_USER

#include <lib_set_logs.h>

#include <sys/select.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

/**
 * All possible representations
 */
enum rt_class
{
   RT_MSC  = 1,
   RT_SDL  = 2,
   RT_VCD =  4,
};

struct rt_object;


/**
 * Each command received from network is extracted into a structure like this one.
 *
 */
struct rt_msg
{
    int                msc_level;  /// untimed msc level
    int                vcd_level;  /// untimed vcd level
    rt_time_t          off;        /// time for asynchronous reception
    int                fid;        /// file descriptor of the source
    struct rt_object * obj1;
    struct rt_object * obj2;
    struct rt_object * group;
    enum rt_class      class;      /// class to which belong the message

    /* message parameters */
    rt_cmd_t           cmd;  
    object_id_t        gid;
    object_id_t        id1;
    object_id_t        id2;
    rt_time_t          time;       /// original timestamp from the sender
    char               text[RT_CFG_MAX_TEXT_LEN];

    // correlated message if any
    struct rt_msg    * corr;       /// for msc messages only
    list_node_t        node;       /// messages are queued from older one to more recent one in a queue.
};

/**
 * object type
 */
typedef enum object_type
{
   RT_NONE    = 0,
   RT_TASK    = 1 << 0,
   RT_MUTEX   = 1 << 1,
   RT_OBJECT  = 1 << 2,
   RT_REAL    = 1 << 3,
   RT_REG     = 1 << 4,
   RT_PARAM   = 1 << 5,
   RT_WIRE    = 1 << 6,
   RT_BOOL    = 1 << 7,
   RT_TIME    = 1 << 8,
   RT_EVENT   = 1 << 9,
   RT_STRING  = 1 << 10,
   RT_INT     = 1 << 11,
   RT_GROUP   = 1 << 12,
}
object_type_t;

/**
 * mutex or task status
 */
typedef enum object_status
{
   RT_OBJECT_INIT,
   RT_OBJECT_READY,
   RT_OBJECT_PREEMPT,
   RT_OBJECT_WAIT,
   RT_OBJECT_RUN
}
object_status_t;

/**
 * msc mark granularity
 */
typedef enum msc_mark_granularity
{
   MSC_MARK_GRANULARITY_NONE,
   MSC_MARK_GRANULARITY_PAGE,
   MSC_MARK_GRANULARITY_LEVEL
}
msc_mark_granularity_t;

/**
 * msc mark display
 */
typedef enum msc_mark_display
{
   MSC_MARK_DISPLAY_NONE,
   MSC_MARK_DISPLAY_REALTIME,
   MSC_MARK_DISPLAY_LEVEL,
   MSC_MARK_DISPLAY_BOTH
}
msc_mark_display_t;

/**
 * log level
 */
int rt_log_level = DEBUG_ERROR;

/**
 * Variant type to store the value of an object. Some values are dynamic pointeur to strings
 */
typedef size_t object_value_t;

/**
 * Each instance of task/mutex/object statically or dynamically created will have a temporar instance of 'rt_object' until the destruction
 * of the object. Objects are used for two purposes:
 * 1- instance checking when a message references an instance. Checking that in the code will allow to run tools that geerate the documentation
 *    without any error.
 * 2- msc page breaks, we need to recreate on page breaks every instances that we created. Moreovern the status of the instance must be reactivated
 */
struct rt_object
{
   object_type_t      type;                      /// object kind of
   int                fid;                       /// origin file identifier of the source that created the objects
   object_id_t        oid;                       /// original object identifier. Always local to fid
   object_value_t     value;                     /// the value of a object, must be kept between page breaks
   object_status_t    status;                    /// object status, for RT_TASK and RT_OBJECT
   int                quantification;            /// size of the object in bits, for fixed size objects
   char               name[RT_CFG_MAX_TEXT_LEN]; /// object name
   char               key[RT_CFG_MAX_TEXT_LEN];  /// object name without any space
   struct rt_object * group;                     /// parent group, if any. NULL means at the 'top'
   list_node_t        node;                      /// objects are queued in the order they werre created. 
   list_node_t        list;                      /// if object is a group, this is a list of sub groups or objects
   int                zombie;                    /// deleted objects become zombie before being removed.
   int                global;                    /// 1 if this object as a global scope.
   object_id_t        global_id;                 /// global object identifier, at system level, independantly of the fid
};

/**
 * a user function called each time we enter inside an object and outgoing from it
 * if this function return 1, the function must stop
 */
typedef int (*iterate_object_t)(struct rt_object *k, int exit, void * info);

int for_each_object(struct rt_object * obj, iterate_object_t action, void * info);

void exec_cmd(struct rt_msg * m);

/**
 * global file descriptors
 */
int msc_fd = -1;
int vcd_fd = -1;
int sdl_fd = -1;

/**
 * when msc_dyn option is set, we need a separate descriptor for vcd definitions
 */
int vcd_def_fd = -1;

/**
 * global outputs enable
 */
int msc_out = 0;
int vcd_out = 0;
int sdl_out = 0;
int vcd_def_out = 0;

/**
 * End of vcd definition section
 */
int vcd_def_end = 0;

/**
 * List of rt_msg queued before beeing processed
 */
list_node_t rt_queue;

/**
 * List of created objects, that we must recreate after each page break
 */
struct rt_object top;

/**
 * Frequency at which the rt_time is working
 */
long int rt_freq = 1000L; /// 1ms

/**
 * number of rt_time levels between two flushes, that is (rt_freq x 1000) / delay_ms
 */
rt_time_t rt_queue_flush = 10;

/**
 * current timed or untimed level for msc.
 */
rt_time_t msc_level = 0;

/**
 * current page level
 */
rt_time_t msc_page = 0;

/**
 * maximum level we can exhib one one page
 */
rt_time_t msc_page_max_levels = 30;

/**
 * height, in mm, of an msc level
 */
int msc_level_height = 10;

/**
 * len of the boxes
 */
int msc_box_height = 8;

/**
 * distance, in mm, between two instances
 */
int msc_inst_dist = 30;

/**
 * number of instances in the current page
 */
int msc_page_instances = 0;

/**
 * maximum number of msc instances per page in the whole document
 */
int msc_max_instances = 0;

/**
 * msc mark granularity
 */
msc_mark_granularity_t msc_mark_grain = MSC_MARK_GRANULARITY_PAGE;

/**
 * msc mark granularity
 */
msc_mark_display_t msc_mark_disp = MSC_MARK_DISPLAY_BOTH;

/**
 * current timed or untimed level for msc.
 */
rt_time_t vcd_level = 0;

/**
 * When this option is enabled (by default it is enabled), levels in msc diagrams are incremented by one step only.
 * The consequence is that time has no meaning, but partial ordering is concerved.
 */
rt_time_t msc_untimed = 0;

/**
 * When this option is enabled (by default it is not), levels in vcd diagrams are incremented by one step only.
 * The consequence is that time has no meaning, but partial ordering is concerved.
 */
rt_time_t vcd_untimed = 0;

/**
 * When turning vcd dump to fifo mode, the defintion of symbols must preceed any value change. This mode helps tools like gtkwaves to reduce
 * consumed memory. The advantage is that gtwave can be run while the simulation continues, but the drawback it that ALL instances must have been declared
 * BEFORE value change.
 */
int vcd_fifo = 0;

/**
 * Return a string for object type
 */
const char * rt_type_name(object_type_t type)
{
   switch(type)
   {
      case RT_NONE:
         return "NONE";
      case RT_TASK:
         return "TASK";
      case RT_MUTEX:
         return "MUTEX";
      case RT_OBJECT:
         return "OBJECT";
      case RT_REAL:
         return "RT_REAL";
      case RT_REG:
         return "RT_REG";
      case RT_PARAM:
         return "RT_PARAM";
      case RT_WIRE:
         return "RT_WIRE";
      case RT_BOOL:
         return "RT_BOOL";
      case RT_TIME:
         return "RT_TIME";
      case RT_EVENT:
         return "RT_EVENT";
      case RT_STRING:
         return "RT_STRING";
      case RT_INT:
         return "RT_INT";
      default:
         return "?TYPE";
   }
}

/**
 * Convert an integer to binary format.
 * Warning, this function only supports little endian processor
 */
void to_binary(unsigned int value, char * result)
{
   int i,j;
   i=0;
   for (j = 31; j >= 0; j--)
   {
      if(value & (1 << j))
      {
         result[i++]='1';
      }
      else if((i > 0) || (j == 0))
      {
         result[i++]='0';
      }
   }
   result[i] = 0;
}

/**
 * remove of separators from a text and copy it into an new string
 */
void generate_key(char * key, const char * text)
{
   int i, n = string_len(text)+1;
   for(i=0; i<n; i++)
   {
      if(text[i] == ' ' || text[i] == '\t')
         key[i]='_';
      else
         key[i] = text[i];
   }
}

/**
 * Return the level, timed or untimed, of a msc message, depending on the mode
 */
rt_time_t msc_get_time(struct rt_msg * m)
{
   if(msc_untimed)
      return m->msc_level;
   else
      return m->time;
}

/**
 * Return the level, timed or untimed, of a vcd message, depending on the mode
 */
rt_time_t vcd_get_time(struct rt_msg * m)
{
   if(vcd_untimed)
      return m->vcd_level;
   else
      return m->time;
}

/**
 * Internal structure for find_object method
 */
struct find_info
{
   int                fid;        // file id to find, if not -1
   object_id_t        oid;        // object id to find
   struct rt_object * group;      // matching group
   const char       * name;
   object_type_t      type;
   int                global;     // find also among global objects
   struct rt_object * found_obj;  // the object found, it exists
};

/**
 * An iterator function for the find_object method
 */
static int find_object_iterator(struct rt_object *k, int exit, void * data)
{
   struct find_info * inf = ((struct find_info *)data);

   if((!k->zombie)                                                             // find among existing objects
   && (((k->oid == inf->oid) && (k->fid == inf->fid))       // source processor file matching is optional
     ||((k->global) && (inf->global) && (k->global_id == inf->oid))))                        // source processor file matching is optional
   {
      inf->found_obj = k;
      return 1;
   }
   else
   {
      return 0;
   }
}


/**
 * An iterator function for a reusable object with the same name, group, fid, quantif and type
 */
static int find_reusable_object_iterator(struct rt_object *k, int exit, void * data)
{
   struct find_info * inf = ((struct find_info *)data);

    if((k->zombie)
    &&((inf->fid == -1) || (k->fid == inf->fid))      // source processor file matching is optional
    && (inf->type == k->type) 
    && (inf->group == k->group) 
    && (string_cmp(inf->name, k->name) == 0))
   {
      inf->found_obj = k;
      return 1;
   }
   else
   {
      return 0;
   }
}


/**
 * find an object given its oid and fid.
 * If globally field is set and the serach fails locally first, then a second pass is done ignoring the fd field
 */
struct rt_object * find_object(int fid, object_id_t oid, int globally)
{
   struct find_info inf;

   inf.fid       = fid;
   inf.oid       = oid;
   inf.found_obj = NULL;
   inf.global    = globally;

   // VERB("find obj %x zombie=%d\n", oid, zombies);
   for_each_object(&top, find_object_iterator, &inf);

   return inf.found_obj;
}

/**
 * find a zombie object having the same type, name, fid and group
 */
struct rt_object * find_reusable_object(int fid, object_id_t oid, const char * name, object_type_t type, struct rt_object * group)
{
   struct find_info inf;

   inf.fid       = fid;
   inf.name      = name;
   inf.type      = type;
   inf.group     = group;
   inf.found_obj = NULL;

   // VERB("find obj %x zombie=%d\n", oid, zombies);
   for_each_object(&top, find_reusable_object_iterator, &inf);

   return inf.found_obj;
}



/**
 * low level log handler function
 */
void rtsv_log_handler(int severity, int output, char * text)
{
   if(severity <= rt_log_level)
   {
      if (output == LOG_DEF_USER)
         fprintf(stdout, "%s", text);
      else
         log_output_ext_def(severity, output, text);
   }
}


/**
 * Initialize object parameters + set the default state and value of a object depending on its type
 */
void init_object(struct rt_object * obj, const char * name, int fid, object_id_t oid, object_type_t type, struct rt_object * group, int light)
{
   obj->oid        = oid;
   obj->status     = RT_OBJECT_INIT;
   obj->zombie     = 0;
   obj->global     = 0;
   obj->global_id  = 0;

   if(light == 0)
   {
      obj->fid   = fid;
      obj->type  = type;
      obj->group = group;

      string_cpy(obj->name, name);
      generate_key(obj->key, name);

      list_init(&obj->list);

      // this object is added tail to the group object list (except for top object where goup is
      // NULL)
      if (group)
         list_add_tail(&obj->node, &group->list);
   }

   switch(obj->type)
   {
      case RT_OBJECT:
      case RT_TASK:
      case RT_STRING:
         if(light == 0)
            obj->value = (size_t)heap_alloc(RT_CFG_MAX_TEXT_LEN);

         if(obj->value)
            string_cpy((char *)obj->value, "UNDEF");
         break;
      case RT_REAL:
      case RT_PARAM:
      case RT_REG:
      case RT_TIME:
      case RT_WIRE:
      case RT_BOOL:
      case RT_INT:
      case RT_EVENT:
         obj->value = (size_t)NAN;
         break;
      default:
         obj->value = 0;
         break;
   }
}

/**
 * reset object:
 * Remove the object from the parent list
 * Also remove dynamically allocated memory for RT_STRING, RT_TASK and RT_OBJECT
 */
void reset_object(struct rt_object * obj)
{
   // remove the object from the group list or from the top list
   list_delete(&obj->node);
   switch(obj->type)
   {
      case RT_OBJECT:
      case RT_TASK:
      case RT_STRING:
         if(obj->value)
            heap_free((void *)obj->value);
         break;
      default:
         break;
   }

}

/**
 * add one object to object list.
 * All objects, any type counfounded, must have a unique identifier (required by msc latex).
 */
struct rt_object * add_object(int fid, object_id_t oid, object_type_t type, struct rt_object * group, char * name)
{
   struct rt_object * obj;
   int light = 0;
   obj = find_object(fid, oid, 0); // object cannot exists on the same fid/oid, but can exists globally
   if(obj)
   {
      ERROR("Cannot add object '%s' with existing identifier fid=%x oid %x zombie=%d\n", name, obj->fid, obj->oid, obj->zombie);
      return NULL;
   }
   else
   {
      // object not found, but may be a zombie can be reused
      obj = find_reusable_object(fid, oid, name, type, group);
      if(obj)
      {
         VERB("reuse zombie object '%s' with identifier fid=%x oid %x\n", name, obj->fid, obj->oid);
         light = 1;
      }
      else
      {
         obj = (struct rt_object *)heap_alloc(sizeof(struct rt_object));
      }
   }

   if(obj)
   {
      init_object(obj, name, fid, oid, type, group, light);
      VERB("add object '%s' fid=%x oid %x\n", obj->name, obj->fid, obj->oid);
   }
   else
   {
      ERROR("cannot allocate memory for object '%s', oid=%x\n", name, oid);
   }
   return obj;
}

/**
 * Delete an object, by removing it from its list, and from physical memory
 */
void del_object(struct rt_object * obj)
{
   reset_object(obj);
   heap_free(obj);
}

/**
 * Remove one object from the list
 * return -1 if an error occured, 0 otherwise
 */
int del_object_by_id(int fid, object_id_t oid, int zombie)
{
   struct rt_object * obj;
   obj = find_object(fid, oid, 0);
   if(obj == NULL)
   {
      ERROR("cannot del object with oid %x, not found\n", oid);
      return -1;
   }

   VERB("del object '%s' fid=%x oid %x (zombie <= %d)\n", obj->name, obj->fid, obj->oid, zombie);
   if(zombie)
      obj->zombie = 1;
   else
      del_object(obj);

   return 0;
}

/**
 * remove all created objects starting fom child to the top (not including it)
 */
static int remove_iterator(struct rt_object *k, int exit, void * data)
{
   if(exit && (k != &top))
      del_object(k);

   return 0;
}


/**
 * send one text line to a text file
 */
int write_line(int fd, char * format, ...)
{
   va_list args;
   char str[RT_CFG_MAX_COMMAND_LEN];

   if(fd < 0)
      return -1;

   va_start (args, format);
   string_vprintf(str, format, args);
   va_end (args);
   return write(fd, str, string_len(str));
}

/**
 * init msc latex document
 */
int msc_new_doc(int fd)
{
   write_line(fd, "\\documentclass{article}\n");
   write_line(fd, "\\usepackage{msc}\n");
   write_line(fd, "\\usepackage{geometry}\n");
   write_line(fd, "\\geometry{paperwidth=PAPERWIDTHmm, paperheight=PAPERHEIGHTmm}\n");
   write_line(fd, "\\geometry{top=1cm, bottom=1cm, left=1cm , right=1cm}\n");
   write_line(fd, "\\begin{document}\n");
   return 0;
}

/**
 * init vcd document
 */
int vcd_new_doc(int fd, char * title)
{
   long int time_scale = 0;
   const char * time_unit;

   if(rt_freq > 1000000) 
   {
      time_scale = 1000000000 / rt_freq;
      time_unit = "ns";
   }
   else if(rt_freq > 1000) 
   {
      time_scale = 1000000 / rt_freq;
      time_unit = "us";
   }
   else
   {
      time_scale = 1000 / rt_freq;
      time_unit = "ms";
   }

   write_line(fd, "$date\n");
   write_line(fd, "   Date text. For example: November 25, 2016.\n");
   write_line(fd, "$end\n");
   write_line(fd, "$comment\n");
   write_line(fd, "%s\n", title);
   write_line(fd, "$end\n");
   write_line(fd, "$timescale %d%s $end\n", time_scale, time_unit);
   return 0;
}

/**
  *  A convenient way to browse easily all objects starting from top
  *     .--------------------------------------.
  *     v                                      |
  *    0)top -> 1)group -> 6)obj -> 7)group --
  *                |  \-----<-----.
  *                v              |
  *             2)obj--> 3)group --
  *                          |   \----<----.
  *                          v             |
  *                        4)obj -> 5)obj --
  */
int for_each_object(struct rt_object * obj, iterate_object_t action, void * info)
{
   struct rt_object * k;
   list_node_t * node;
   list_node_t * tmp;

   // VERB("iter obj %x zombie=%d type=%d name=%s\n", obj->id, obj->zombie, obj->type, obj->name);
   if(action(obj, 0, info))
      return 1;

   list_for_each_safe(node, tmp, &obj->list)
   {
      k = list_entry(node, struct rt_object, node);
      if(for_each_object(k, action, info))
         return 1;
   }

   return action(obj, 1, info);
}

/**
 * An iterator function for msc_dump_start function.
 * Redraw an object following the natural object group order.
 */
int msc_redraw(struct rt_object * k, int exit, void * info)
{
   struct rt_msg m;
   object_status_t status;

   if(exit || k->zombie)
      return 0;

   // values that we don't care
   m.time = 0;
   m.id2 = 0;
   m.msc_level = 0;
   m.vcd_level = 0;
   m.group = NULL;
   m.obj1 = NULL;
   m.obj2 = NULL;

   /**
    * redraw this object instance
    */
   switch (k->type)
   {
      case RT_TASK:
         m.cmd = RT_DEF_CMD_DECLTASK;
         break;
      case RT_MUTEX:
         m.cmd = RT_DEF_CMD_DECLMUTEX;
         break;
      case RT_OBJECT:
         m.cmd = RT_DEF_CMD_DECLOBJ;
         break;
      default:
         // other objects are not not msc
         return 0;
   }
   m.id1  = k->oid;
   m.obj1 = k;
   string_cpy(m.text, k->name);
   exec_cmd(&m);

   /**
    * restore object status (when msc page breaks, all values are drawed as defaut,
    * RT_OBJECT_READY)
    */
   status    = k->status;
   k->status = RT_OBJECT_INIT;
   switch (status)
   {
      case RT_OBJECT_PREEMPT:
         m.cmd = RT_DEF_CMD_PREEMPT;
         break;
      case RT_OBJECT_RUN:
         m.cmd = RT_DEF_CMD_RUN;
         break;
      case RT_OBJECT_WAIT:
         m.cmd = RT_DEF_CMD_WAIT;
         break;
      case RT_OBJECT_READY:
         m.cmd = RT_DEF_CMD_READY;
         break;
      default:
         // other objects are not not msc
         return 0;
   }
   exec_cmd(&m);
   return 0;
}

/**
 * end current msc diagram
 */
int msc_dump_start(int fd, char * title, rt_time_t time)
{
   write_line(fd, "\\begin{msc}{%s}\n", title);
   write_line(fd, "\\setlength{\\topheaddist}{%dmm}\n",      msc_level_height);
   write_line(fd, "\\setlength{\\levelheight}{%dmm}\n",      msc_level_height);
   write_line(fd, "\\setlength{\\bottomfootdist}{%dmm}\n",   msc_level_height);
   write_line(fd, "\\setlength{\\bottomfootdist}{%dmm}\n",   msc_level_height);
   write_line(fd, "\\setlength{\\actionheight}{%dmm}\n",     msc_box_height);
   write_line(fd, "\\setlength{\\conditionheight}{%dmm}\n",  msc_box_height);
   write_line(fd, "\\setlength{\\instheadheight}{%dmm}\n",   msc_box_height);
   write_line(fd, "\\setlength{\\firstlevelheight}{%dmm}\n", msc_box_height);
   write_line(fd, "\\setlength{\\lastlevelheight}{%dmm}\n",  msc_box_height);
   write_line(fd, "\\setlength{\\instdist}{%dmm}\n",         msc_inst_dist);
   write_line(fd, "\\setlength{\\envinstdist}{\\instdist}\n");
   write_line(fd, "\\setlength{\\instfootheight}{%dmm}\n",   3);
   write_line(fd, "\\setlength{\\markdist}{%dmm}\n",         0);

   msc_page_instances = 0;

   for_each_object(&top, msc_redraw, NULL);

   if(msc_mark_grain == MSC_MARK_GRANULARITY_PAGE) {
      switch(msc_mark_disp) {
         case MSC_MARK_DISPLAY_NONE:
            break;
         case MSC_MARK_DISPLAY_BOTH:
            write_line(fd, "\\mscmark[bl]{%d : %d}{envleft}\n", time, msc_level);
            break;
         case MSC_MARK_DISPLAY_REALTIME:
            write_line(fd, "\\mscmark[bl]{%d}{envleft}\n", time);
            break;
         case MSC_MARK_DISPLAY_LEVEL:
            write_line(fd, "\\mscmark[bl]{%d}{envleft}\n", msc_level);
            break;
      }
   }

   return 0;
}

/**
 * An iterator function for vcd_start_dump function.
 * Redraw an object following the natural object group order.
 */
int vcd_define_symbols(struct rt_object * k, int exit, void * info)
{
   struct rt_msg m;

   rt_cmd_t cmd;

   // values that we don't care
   m.time = 0;
   m.id2 = 0;
   m.msc_level = 0;
   m.vcd_level = 0;
   m.obj1 = NULL;
   m.obj2 = NULL;

   // if the declaration of the vcd symbol is already in the vcd_def file, do not redeclare it
   if(exit)
   {
      if(k->type == RT_GROUP)
         cmd = RT_DEF_CMD_DELGRP;
      else
         return 0;
   }
   else
   {
      switch (k->type)
      {
         case RT_REAL:
            cmd = RT_DEF_CMD_DECLREAL;
            break;
         case RT_REG:
            cmd = RT_DEF_CMD_DECLREG;
            break;
         case RT_PARAM:
            cmd = RT_DEF_CMD_DECLPARAM;
            break;
         case RT_WIRE:
            cmd = RT_DEF_CMD_DECLWIRE;
            break;
         case RT_BOOL:
            cmd = RT_DEF_CMD_DECLBOOL;
            break;
         case RT_TIME:
            cmd = RT_DEF_CMD_DECLTIME;
            break;
         case RT_EVENT:
            cmd = RT_DEF_CMD_DECLEVENT;
            break;
         case RT_STRING:
            cmd = RT_DEF_CMD_DECLSTRING;
            break;
         case RT_INT:
            cmd = RT_DEF_CMD_DECLINT;
            break;
         case RT_TASK:
            cmd = RT_DEF_CMD_DECLTASK;
            break;
         case RT_OBJECT:
            cmd = RT_DEF_CMD_DECLOBJ;
            break;
         case RT_MUTEX:
            cmd = RT_DEF_CMD_DECLMUTEX;
            break;
         case RT_GROUP:
            cmd = RT_DEF_CMD_CREATGRP;
            break;
         default:
            return 0;
      }
   }

   m.id1 = k->oid;
   m.id2 = k->quantification;
   m.cmd = cmd;
   string_cpy(m.text, k->name);
   m.obj1 = k;
   exec_cmd(&m);

   return 0;
}

/**
 * Write vcd definitions
 */
void vcd_write_definitions()
{
   int saved_msc = msc_out;
   int saved_sdl = sdl_out;

   msc_out = 0;
   sdl_out = 0;

   INFO("end of vcd definition section\n");
   vcd_def_out = 1;
   for_each_object(&top, vcd_define_symbols, NULL);
   vcd_def_out = 0;

   msc_out = saved_msc;
   sdl_out = saved_sdl;
}

/**
 * An iterator function for vcd_start_dump function.
 * Redraw an object following the natural object group order.
 */
int vcd_reload_values(struct rt_object * k, int exit, void * info)
{
   struct rt_msg m;

   rt_cmd_t cmd;

   if(exit)
      return 0;

   // values that we don't care
   m.time = 0;
   m.id2 = 0;
   m.msc_level = 0;
   m.vcd_level = 0;
   m.obj1 = NULL;
   m.obj2 = NULL;

   switch (k->type)
   {
      case RT_REAL:
         cmd = RT_DEF_CMD_SETREAL;
         break;
      case RT_REG:
         cmd = RT_DEF_CMD_SETREG;
         break;
      case RT_PARAM:
         cmd = RT_DEF_CMD_SETPARAM;
         break;
      case RT_WIRE:
         cmd = RT_DEF_CMD_SETWIRE;
         break;
      case RT_BOOL:
         cmd = RT_DEF_CMD_SETBOOL;
         break;
      case RT_TIME:
         cmd = RT_DEF_CMD_SETTIME;
         break;
      case RT_EVENT:
         cmd = RT_DEF_CMD_SETEVENT;
         break;
      case RT_STRING:
         cmd = RT_DEF_CMD_SETSTRING;
         break;
      case RT_INT:
         cmd = RT_DEF_CMD_SETINT;
         break;
      case RT_TASK:
      case RT_OBJECT:
         cmd = RT_DEF_CMD_SETSTATE;
         break;
      default:
         return 0;
   }
   m.id1  = k->oid;
   m.cmd  = cmd;
   m.obj1 = k;

   switch (k->type)
   {
      case RT_STRING:
      case RT_TASK:
      case RT_OBJECT:
         m.id2 = 0;
         string_cpy(m.text, (char *)k->value);
         break;
      default:
         m.id2 = k->value;
         string_cpy(m.text, "");
         break;
   }
   exec_cmd(&m);

   /**
    * restore object status (when msc page breaks, all values are drawed as defaut,
    * RT_OBJECT_READY)
    */
   if (k->type == RT_TASK || k->type == RT_OBJECT)
   {
      object_status_t status = k->status;

      k->status = RT_OBJECT_INIT; // force redraw of the status

      switch (status)
      {
         case RT_OBJECT_PREEMPT:
            m.cmd = RT_DEF_CMD_PREEMPT;
            break;
         case RT_OBJECT_RUN:
            m.cmd = RT_DEF_CMD_RUN;
            break;
         case RT_OBJECT_WAIT:
            m.cmd = RT_DEF_CMD_WAIT;
            break;
         case RT_OBJECT_READY:
            m.cmd = RT_DEF_CMD_READY;
            break;
         case RT_OBJECT_INIT:
            return 0;
      }

      exec_cmd(&m);
   }
   return 0;
}

/**
 * at the time we reenable the logs, all values must be updated with the internal values stored while the logs
 * where disabled.
 */
int vcd_dump_start(int fd, rt_time_t time)
{
   int saved_msc = msc_out;
   int saved_sdl = sdl_out;

   msc_out = 0;
   sdl_out = 0;

   // indicate the new time where dump restart
   if(vcd_level > 0)
      write_line(vcd_fd, "#%d\n", vcd_level);

   // restore current values
   for_each_object(&top, vcd_reload_values, NULL);

   msc_out = saved_msc;
   sdl_out = saved_sdl;

   return 0;
}

/**
 * end current msc diagram
 */
int msc_dump_stop(int fd, rt_time_t time)
{
   if(msc_mark_grain == MSC_MARK_GRANULARITY_PAGE) {
      switch(msc_mark_disp) {
         case MSC_MARK_DISPLAY_NONE:
            break;
         case MSC_MARK_DISPLAY_BOTH:
            write_line(fd, "\\mscmark[tl]{%d : %d}{envleft}\n", time, msc_level);
            break;
         case MSC_MARK_DISPLAY_REALTIME:
            write_line(fd, "\\mscmark[tl]{%d}{envleft}\n", time);
            break;
         case MSC_MARK_DISPLAY_LEVEL:
            write_line(fd, "\\mscmark[tl]{%d}{envleft}\n", msc_level);
            break;
      }
   }

   write_line(fd, "\\end{msc}\n");

   if(msc_page_instances > msc_max_instances)
      msc_max_instances = msc_page_instances;

   return 0;
}


/**
 * end current document
 */
int msc_end_doc(int fd)
{
   write_line(fd, "\\end{document}\n");
   return 0;
}

/**
 * read one block of binary data from a binary file.
 * Each data block has a 8 bit header indicating the length of the data block that follow.
 */
int read_data(int fd, char * buffer, size_t max)
{
   int rem_len, len;
   int rc;

   /* read header length */
   if(read(fd, buffer, 1) < 0)
      return -1;

   len = rem_len = buffer[0];

   if((rem_len == 0) || (rem_len > max))
      return -1;

   while(rem_len > 0)
   {
      rc = read(fd, buffer, rem_len);
      if(rc <= 0) 
         return -1;

      buffer += rc;
      rem_len -= rc;
   }

   return len;
}

/**
 * read one line in a text file. The line is supposed to finished with '\n'. This character is replaced by a zero instead.
 * if max is reached before the end of the file, return the read len
 * if end of file is reached, return the actual len, but the string is not NULL terminated
 * else return a null terminated string, the '\n' havinf being removed. 
 */
int read_line(int fd, char * buffer, size_t max)
{
   int len = 0;
   int rc;

   while(len < max)
   {
      rc = read(fd, buffer, 1);
      if(rc <= 0) 
         break;

      len += 1;

      if(*buffer == '\n')
      {
         *buffer = '\0';
         break;
      }

      buffer++;
   }
   return len;
}


/**
 * Display the content of a message.
 * For debug only.
 */
void print_msg(struct rt_msg * m)
{
   fprintf(stdout, "cmd  %d (%s)", m->cmd, rt_cmd_name(m->cmd));
   fprintf(stdout, ", time %d", m->time);
   fprintf(stdout, ", gid  %lx", m->gid);
   fprintf(stdout, ", fid  %x", m->fid);
   fprintf(stdout, ", id1  %lx", m->id1);
   fprintf(stdout, ", id2  %lx", m->id2);
   fprintf(stdout, ", text '%s'\n", m->text);
}

/**
 * Find a simular message to the one passed as argument
 * - id1 and id2 must match
 * - text must match
 *
 * Note: the cmd don't care
 */
struct rt_msg * msc_find_msg(struct rt_msg * m)
{
   list_node_t * node;
   struct rt_msg * k;
   list_for_each(node, &rt_queue)
   {
      k = list_entry(node, struct rt_msg, node);
      if ((k->id1 == m->id1) 
       && (k->id2 == m->id2)
       && (string_cmp(k->text, m->text) == 0))
      {
         return k;
      }
   }
   return NULL;
}

void exec_decltask(struct rt_msg * m)
{
   m->obj1->quantification = 0;
   if(msc_out)
   {
      msc_page_instances++;
      write_line(msc_fd, "\\declinst{%x}{task}{%s}\n", m->obj1, m->text);
   }
   if(sdl_out)
   {
   }
   if(vcd_def_out)
   {
      // status of the process at os level
      write_line(vcd_def_fd, "$var wire 1 ^%x y_%s $end\n", m->obj1, m->obj1->key);

      // state of the process
      write_line(vcd_def_fd, "$var string 0 $%x %s $end\n", m->obj1, m->obj1->key);
   }
}

void exec_declmutex(struct rt_msg * m)
{
   if(msc_out)
   {
      msc_page_instances++;
      write_line(msc_fd, "\\declinst{%x}{mutex}{%s}\n", m->obj1, m->text);
   }
   if(sdl_out)
   {
   }
   if(vcd_def_out)
   {
      // status of the mutex
      write_line(vcd_def_fd, "$var wire 1 ^%x %s $end\n", m->obj1, m->obj1->key);
   }
}

void exec_declobj(struct rt_msg * m)
{
   char *p = m->text;
   char *inst_name;
   char *inst_type;
   inst_type = string_sep(&p, "\t ");
   inst_name = string_sep(&p, "\t ");
   m->obj1->quantification = 0;
   if(msc_out)
   {
      msc_page_instances++;
      write_line(msc_fd, "\\declinst{%x}{%s}{%s}\n", m->obj1, inst_type, inst_name);
   }
   if(sdl_out)
   {
   }
   if(vcd_def_out)
   {
      // status of the object
      write_line(vcd_def_fd, "$var wire 1 ^%x y_%s $end\n", m->obj1, inst_name);

      // state of the object.
      write_line(vcd_def_fd, "$var string 0 $%x %s $end\n", m->obj1, inst_name);
   }
}

/**
 * find a similar message.
 * Apply only to the following messages:
 *    - RT_DEF_CMD_SENDMSG
 *    - RT_DEF_CMD_SETTIMER
 * If message is correlated to one message of the queue, the following fields are changed:
 *    - m->off is set to k->time - m->time
 *    - m->corr is set to k
 *    - k->corr is set to m
 *    - k->off is set to - m->off
 *    - 1 is returned
 * Else:
 *    - 0 is returned
 */
int msc_find_corr(struct rt_msg * m)
{
   struct rt_msg * k;

   if ((m->cmd == RT_DEF_CMD_SENDMSG) || (m->cmd == RT_DEF_CMD_SETTIMER))
   {
      k = msc_find_msg(m);

      // if message is lost, offset is kept to -1
      if (k)
      {
         // logical order: the received message should have a reception time more recent (or =)
         if (k->time < m->time)
         {
            ERROR("logical condition broken\n");
            ERROR(" => recvmsg:\n");
            print_msg(k);
            ERROR(" => sendmsg:\n");
            print_msg(m);
         }
         else
         {
            m->off  = msc_get_time(k) - msc_get_time(m);
            k->off  = -m->off;
            k->corr = m;
            m->corr = k;
            return 1;
         }
      }
   }
   return 0;
}

void exec_sendmsg(struct rt_msg * m)
{
   if(msc_out)
   {
      if(m->corr == NULL)
         write_line(msc_fd, "\\lost[r]{%s}{}{%x}\n", m->text, m->obj1);
      else
         write_line(msc_fd, "\\mess{%s}{%x}[0.1]{%x}[%d]\n", m->text, m->obj1, m->obj2, m->off);
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
   }
}

void exec_call(struct rt_msg * m)
{
   if(msc_out)
   {
      write_line(msc_fd, "\\mess{%s}{%x}{%x}\n", m->text, m->obj1, m->obj2);
      if(m->obj2->status != RT_OBJECT_RUN)
         write_line(msc_fd, "\\regionstart{activation}{%x}\n", m->obj2);
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
      if(m->obj2->status != RT_OBJECT_RUN)
         write_line(vcd_fd, "1^%x $end\n", m->obj2);
   }
   m->obj2->status = RT_OBJECT_RUN;
}

void exec_recvmsg(struct rt_msg * m)
{
   if(msc_out)
   {
      if(m->corr == NULL)
         write_line(msc_fd, "\\found[r]{%s}{}{%x}\n", m->text, m->obj1);
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
   }
}

void exec_sleep(struct rt_msg * m)
{
   if(msc_out)
   {
      if(m->obj1->status != RT_OBJECT_WAIT)
         write_line(msc_fd, "\\regionstart{coregion}{%x}\n", m->obj1);
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
      if(m->obj1->status != RT_OBJECT_WAIT)
         write_line(vcd_fd, "1^%x $end\n", m->obj1);
   }

   m->obj1->status = RT_OBJECT_WAIT;
}

void exec_switch(struct rt_msg * m)
{
   if(msc_out)
   {
      write_line(msc_fd, "\\mess*{switch}{%x}{%x}\n", m->obj1, m->obj2);
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
   }
}

void exec_return(struct rt_msg * m)
{
   if(msc_out)
   {
      write_line(msc_fd, "\\order{%x}{%x}\n", m->obj1, m->obj2);
      if(m->obj1->status == RT_OBJECT_RUN)
         write_line(msc_fd, "\\regionend{%x}\n", m->obj1);
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
      if(m->obj1->status == RT_OBJECT_RUN)
         write_line(vcd_fd, "0^%x $end\n", m->obj1);
   }
   m->obj1->status = RT_OBJECT_READY;
}

void exec_comment(struct rt_msg * m)
{
   if(msc_out)
   {
      write_line(msc_fd, "\\msccomment[%c]{%s}{%x}\n", 'r', m->text, m->obj1);
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
   }
}

void exec_action(struct rt_msg * m)
{
   if(msc_out)
   {
      write_line(msc_fd, "\\action*{%s}{%x}\n", m->text, m->obj1);
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
   }
}

void exec_settimer(struct rt_msg * m)
{
   if(msc_out)
   {
      if(m->corr == NULL)
         write_line(msc_fd, "\\settimer[r]{%s}{%x}\n", m->text, m->obj1);
      else if(m->corr->cmd == RT_DEF_CMD_TIMEOUT)
         write_line(msc_fd, "\\settimeout[r]{%s}{%x}[%d]\n", m->text, m->obj1, m->off);
      else if(m->corr->cmd == RT_DEF_CMD_STOPTIMER)
         write_line(msc_fd, "\\setstoptimer[r]{%s}{%x}[%d]\n", m->text, m->obj1, m->off);
   }

   if(msc_out)
   {
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
   }
}

void exec_exptimer(struct rt_msg * m)
{
   if(msc_out)
   {
      if(m->corr == NULL)
         write_line(msc_fd, "\\timeout[r]{%s}{%x}\n", m->text, m->obj1);
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
   }
}

void exec_stoptimer(struct rt_msg * m)
{
   if(msc_out)
   {
      if(m->corr == NULL)
         write_line(msc_fd, "\\stoptimer[r]{%s}{%x}\n", m->text, m->obj1);
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
   }
}

void exec_ready(struct rt_msg * m)
{
   if(msc_out)
   {
      if(m->obj1->status != RT_OBJECT_READY)
         write_line(msc_fd, "\\regionend{%x}\n", m->obj1);
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
      if(m->obj1->status != RT_OBJECT_READY)
         write_line(vcd_fd, "0^%x $end\n", m->obj1);
   }

   m->obj1->status = RT_OBJECT_READY;
}

void exec_suspend(struct rt_msg * m)
{
   if(msc_out)
   {
      if(m->obj1->status != RT_OBJECT_PREEMPT)
         write_line(msc_fd, "\\regionstart{suspension}{%x}\n", m->obj1);
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
      if(m->obj1->status != RT_OBJECT_PREEMPT)
         write_line(vcd_fd, "x^%x $end\n", m->obj1);
   }
   m->obj1->status = RT_OBJECT_PREEMPT;
}

void exec_creattask(struct rt_msg * m)
{
   if(msc_out)
   {
      msc_page_instances++;
      write_line(msc_fd, "\\dummyinst{%x}\n", m->obj2);
      write_line(msc_fd, "\\create{spawn}[t]{%x}[0.5]{%x}{task}{%s}\n", m->obj1, m->obj2, m->text);
   }
   if(sdl_out)
   {
   }
   if(vcd_def_out)
   {
      // status of the process
      write_line(vcd_def_fd, "$var wire 1 ^%x y_%s $end\n", m->obj2, m->obj2->key);

      // value (user state) of the process
      write_line(vcd_def_fd, "$var string 0 $%x %s $end\n", m->obj2, m->obj2->key);
   }
   if(vcd_out)
   {
      // dynamically created task are ready
      write_line(vcd_fd, "0^%x $end\n", m->obj2);
   }
}

void exec_creatmutex(struct rt_msg * m)
{
   if(msc_out)
   {
      msc_page_instances++;
      write_line(msc_fd, "\\dummyinst{%x}\n", m->obj2);
      write_line(msc_fd, "\\create{}[t]{%x}[0.5]{%x}{mutex}{%s}\n", m->obj1, m->obj2, m->text);
   }
   if(sdl_out)
   {
   }
   if(vcd_def_out)
   {
      // status of the mutex
      write_line(vcd_def_fd, "$var wire 1 ^%x %s $end\n", m->obj2, m->obj2->key);
   }
   if(vcd_out)
   {
      write_line(vcd_fd, "0^%x $end\n", m->obj2);
   }
}

void exec_creatobj(struct rt_msg * m)
{
   char *p = m->text;
   char *inst_name;
   char *inst_type;
   inst_type = string_sep(&p, "\t ");
   inst_name = string_sep(&p, "\t ");
   if(msc_out)
   {
      msc_page_instances++;
      write_line(msc_fd, "\\dummyinst{%x}\n", m->obj2);
      write_line(msc_fd, "\\create{}[t]{%x}[0.5]{%x}{%s}{%s}\n", m->obj1, m->obj2, inst_name, inst_type);
   }
   if(sdl_out)
   {
   }
   if(vcd_def_out)
   {
      // status of the object
      write_line(vcd_def_fd, "$var wire 1 ^%x y_%s $end\n", m->obj2, inst_name);

      // value (user state) of the object.
      write_line(vcd_def_fd, "$var string 0 $%x %s $end\n", m->obj2, inst_name);
   }
   if(vcd_out)
   {
      write_line(vcd_fd, "0^%x $end\n", m->obj2);
   }
}

void exec_take(struct rt_msg * m)
{
   if(msc_out)
   {
      write_line(msc_fd, "\\mess{take}{%x}{%x}\n", m->obj1, m->obj2);
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
   }
}

void exec_give(struct rt_msg * m)
{
   if(msc_out)
   {
      write_line(msc_fd, "\\mess{give}{%x}{%x}\n", m->obj1, m->obj2);
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
   }
}

void exec_delmutex(struct rt_msg * m)
{
   if(msc_out)
   {
      write_line(msc_fd, "\\stop{%x}\n", m->obj2);
      if(m->obj1 != m->obj2)
         write_line(msc_fd, "\\mess{}{%x}{%x}\n", m->obj1, m->obj2);
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
      write_line(vcd_fd, "x^%x $end\n", m->obj2);
   }
}

void exec_deltask(struct rt_msg * m)
{
   if(msc_out)
   {
      write_line(msc_fd, "\\stop{%x}\n", m->obj2);
      if(m->obj1 != m->obj2)
         write_line(msc_fd, "\\mess{kill}{%x}{%x}\n", m->obj1, m->obj2);
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
      write_line(vcd_fd, "x^%x $end\n", m->obj2);
   }
}

void exec_delobj(struct rt_msg * m)
{
   if(msc_out)
   {
      write_line(msc_fd, "\\stop{%x}\n", m->obj2);
      if(m->obj1 != m->obj2)
         write_line(msc_fd, "\\mess{}{%x}{%x}\n", m->obj1, m->obj2);
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
      write_line(vcd_fd, "x^%x $end\n", m->obj2);
   }
}


void exec_declbool(struct rt_msg * m)
{
   m->obj1->quantification = 1;
   if(msc_out)
   {
   }
   if(sdl_out)
   {
   }
   if(vcd_def_out)
   {
      write_line(vcd_def_fd, "$var wire 1 &%x %s $end\n", m->obj1, m->obj1->key);
   }
}

void exec_declwire(struct rt_msg * m)
{
   m->obj1->quantification = m->id2;
   if(msc_out)
   {
   }
   if(sdl_out)
   {
   }
   if(vcd_def_out)
   {
      write_line(vcd_def_fd, "$var wire %d @%x %s $end\n", m->id2, m->obj1, m->obj1->key);
   }
}

void exec_declint(struct rt_msg * m)
{
   m->obj1->quantification = 0;
   if(msc_out)
   {
   }
   if(sdl_out)
   {
   }
   if(vcd_def_out)
   {
      write_line(vcd_def_fd, "$var real 0 #%x %s $end\n", m->obj1, m->obj1->key);
   }
}

void exec_declreal(struct rt_msg * m)
{
   m->obj1->quantification = 0;
   if(msc_out)
   {
   }
   if(sdl_out)
   {
   }
   if(vcd_def_out)
   {
      write_line(vcd_def_fd, "$var real 0 #%x %s $end\n", m->obj1, m->obj1->key);
   }
}

void exec_declstring(struct rt_msg * m)
{
   m->obj1->quantification = 0;
   if(msc_out)
   {
   }
   if(sdl_out)
   {
   }
   if(vcd_def_out)
   {
      write_line(vcd_def_fd, "$var string 0 $%x %s $end\n", m->obj1, m->obj1->key);
   }
}

void exec_declstate(struct rt_msg * m)
{
   m->obj1->quantification = 0;
   if(msc_out)
   {
   }
   if(sdl_out)
   {
   }
   if(vcd_def_out)
   {
      write_line(vcd_def_fd, "$var string 0 $%x %s $end\n", m->obj1, m->obj1->key);
   }
}

void exec_setint(struct rt_msg * m)
{
   if(msc_out)
   {
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
      write_line(vcd_fd, "r%d #%x\n", m->id2, m->obj1);
   }
   m->obj1->value = m->id2;
}

void exec_setreal(struct rt_msg * m)
{
   if(msc_out)
   {
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
      write_line(vcd_fd, "r%d #%x\n", m->id2, m->obj1);
   }
   m->obj1->value = m->id2;
}

void exec_setstring(struct rt_msg * m)
{
   if(msc_out)
   {
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
      char key[RT_CFG_MAX_TEXT_LEN];
      generate_key(key, m->text);
      write_line(vcd_fd, "s%s $%x\n", key, m->obj1);
   }
   string_cpy((char*)m->obj1->value, m->text);
}

void exec_setstate(struct rt_msg * m)
{
   if(msc_out)
   {
      write_line(msc_fd, "\\condition*{%s}{%x}\n", m->text, m->obj1);
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
      char key[RT_CFG_MAX_TEXT_LEN];
      generate_key(key, m->text);
      write_line(vcd_fd, "s%s $%x\n", key, m->obj1);
   }
   string_cpy((char*)m->obj1->value, m->text);
}

void exec_creategrp(struct rt_msg * m)
{
   if(msc_out)
   {
   }
   if(sdl_out)
   {
   }
   if(vcd_def_out)
   {
      write_line(vcd_def_fd, "$scope module %s $end\n", m->obj1->key);
   }
}

void exec_delgrp(struct rt_msg * m)
{
   if(msc_out)
   {
   }
   if(sdl_out)
   {
   }
   if(vcd_def_out)
   {
      write_line(vcd_def_fd, "$upscope $end\n");
   }
}

void exec_activate(struct rt_msg * m)
{
   if(msc_out)
   {
      if(m->obj1->status != RT_OBJECT_RUN)
         write_line(msc_fd, "\\regionstart{activation}{%x}\n", m->obj1);
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
      if(m->obj1->status != RT_OBJECT_RUN)
         write_line(vcd_fd, "1^%x $end\n", m->obj1);
   }
   m->obj1->status = RT_OBJECT_RUN;
}

void exec_acquire(struct rt_msg * m)
{
   if(msc_out)
   {
      write_line(msc_fd, "\\mess*{acquire}{%x}{%x}\n", m->obj1, m->obj2);

      if(m->obj1->status != RT_OBJECT_RUN)
         write_line(msc_fd, "\\regionstart{activation}{%x}\n", m->obj1);

      if(m->obj2->status != RT_OBJECT_READY)
         write_line(msc_fd, "\\regionend{%x}\n", m->obj2);
   }
   if(sdl_out)
   {
   }
   if(vcd_out)
   {
      if(m->obj1->status != RT_OBJECT_RUN)
         write_line(vcd_fd, "1^%x\n", m->obj1);

      if(m->obj2->status != RT_OBJECT_READY)
         write_line(vcd_fd, "0^%x\n", m->obj2);
   }
   m->obj1->status = RT_OBJECT_RUN;
   m->obj2->status = RT_OBJECT_READY;
}

void exec_declevent(struct rt_msg * m)
{
   m->obj1->quantification = 1;
   if(msc_out)
   {
   }
   if(vcd_def_out)
   {
      write_line(vcd_def_fd, "$var event 1 !%x %s $end\n", m->obj1, m->obj1->key);
   }
   if(sdl_out)
   {
   }
}

void exec_decltime(struct rt_msg * m)
{
   m->obj1->quantification = m->id2;
   if(msc_out)
   {
   }
   if(vcd_def_out)
   {
      write_line(vcd_def_fd, "$var time %d @%x %s $end\n", m->id2, m->obj1, m->obj1->key);
   }
   if(sdl_out)
   {
   }
}

void exec_declparam(struct rt_msg * m)
{
   m->obj1->quantification = m->id2;
   if(msc_out)
   {
   }
   if(vcd_def_out)
   {
      write_line(vcd_def_fd, "$var parameter %d @%x %s $end\n", m->id2, m->obj1, m->obj1->key);
   }
   if(sdl_out)
   {
   }
}

void exec_declreg(struct rt_msg * m)
{
   m->obj1->quantification = m->id2;
   if(msc_out)
   {
   }
   if(vcd_def_out)
   {
      write_line(vcd_def_fd, "$var reg %d @%x %s $end\n", m->id2, m->obj1, m->obj1->key);
   }
   if(sdl_out)
   {
   }
}

void exec_setevent(struct rt_msg * m)
{
   if(msc_out)
   {
   }
   if(vcd_out)
   {
      write_line(vcd_fd, "%d!%x\n", m->id2, m->obj1);
   }
   if(sdl_out)
   {
   }
   m->obj1->value = m->id2;
}

void exec_setparam(struct rt_msg * m)
{
   if(msc_out)
   {
   }
   if(vcd_out)
   {
      char bin[32];
      to_binary(m->id2, bin);
      write_line(vcd_fd, "b%s @%x\n", bin, m->obj1);
   }
   if(sdl_out)
   {
   }
   m->obj1->value = m->id2;
}

void exec_setreg(struct rt_msg * m)
{
   if(msc_out)
   {
   }
   if(vcd_out)
   {
      char bin[32];
      to_binary(m->id2, bin);
      write_line(vcd_fd, "b%s @%x\n", bin, m->obj1);
   }
   if(sdl_out)
   {
   }
   m->obj1->value = m->id2;
}

void exec_settime(struct rt_msg * m)
{
   if(msc_out)
   {
   }
   if(vcd_out)
   {
      char bin[32];
      to_binary(m->id2, bin);
      write_line(vcd_fd, "b%s @%x\n", bin, m->obj1);
   }
   if(sdl_out)
   {
   }
   m->obj1->value = m->id2;
}

void exec_setbool(struct rt_msg * m)
{
   if(msc_out)
   {
   }
   if(vcd_out)
   {
      write_line(vcd_fd, "%d&%x\n", m->id2, m->obj1);
   }
   if(sdl_out)
   {
   }
   m->obj1->value = m->id2;
}

void exec_setwire(struct rt_msg * m)
{
   if(msc_out)
   {
   }
   if(vcd_out)
   {
      char bin[32];
      to_binary(m->id2, bin);
      write_line(vcd_fd, "b%s @%x\n", bin, m->obj1);
   }
   if(sdl_out)
   {
   }
   m->obj1->value = m->id2;
}

void exec_startdump(struct rt_msg * m)
{
   if(msc_fd > 0)
   {
      if(msc_out == 0)
      {
         msc_out   = 1;
         msc_page  = msc_get_time(m);
         msc_level = msc_page;
         msc_dump_start(msc_fd, "msc", m->time);
      }
      else
      {
         ERROR("msc dumping already activated\n");
      }
   }
   if(vcd_fd > 0)
   {
      if(vcd_out == 0)
      {
         vcd_out   = 1;
         vcd_level = vcd_get_time(m);
         vcd_dump_start(vcd_fd, m->time);
      }
      else
      {
         ERROR("vcd dumping already activated\n");
      }
 
   }
   if(sdl_fd > 0)
   {
   }
}

void exec_stopdump(struct rt_msg * m)
{
   if(msc_fd > 0)
   {
      if(msc_out)
      {
         msc_dump_stop(msc_fd, m->time);
         msc_out = 0;
      }
      else
      {
         ERROR("msc dumping already deactivated\n");
      }
   }
   if(vcd_fd > 0)
   {
      if(vcd_out)
      {
         vcd_level = vcd_get_time(m);
         write_line(vcd_fd, "#%d\n", vcd_level);
         vcd_out = 0;
      }
      else
      {
         ERROR("vcd dumping already deactivated\n");
      }
   }
   if(sdl_fd > 0)
   {
   }
}

void exec_delvar(struct rt_msg * m)
{
   if(msc_out)
   {
   }
   if(vcd_out)
   {
      switch(m->obj1->type)
      {
         case RT_STRING:
            write_line(vcd_fd, "sUNDEF $%x\n", m->obj1);
            break;
         case RT_INT:
         case RT_REAL:
            write_line(vcd_fd, "rnan #%x\n", m->obj1);
            break;
         case RT_BOOL:
            write_line(vcd_fd, "x&%x\n", m->obj1);
            break;
         case RT_PARAM:
         case RT_WIRE:
         case RT_TIME:
         case RT_REG:
            write_line(vcd_fd, "bx @%x\n", m->obj1);
            break;
         default:
            break;
      }
   }
   if(sdl_out)
   {
   }
}

void exec_setglobal(struct rt_msg * m)
{
   m->obj1->global = 1;
   m->obj1->global_id = m->id2;

   if(msc_out)
   {
   }
   if(vcd_out)
   {
   }
   if(sdl_out)
   {
   }
}

/**
 * Indicate all representations for a command
 */
int classify_cmd(rt_cmd_t cmd)
{
   int class = 0;
   switch(cmd)
   {
      case RT_DEF_CMD_SETEVENT:
      case RT_DEF_CMD_SETPARAM:
      case RT_DEF_CMD_SETREG:
      case RT_DEF_CMD_SETTIME:
      case RT_DEF_CMD_SETBOOL:
      case RT_DEF_CMD_DECLBOOL:
      case RT_DEF_CMD_DECLWIRE:
      case RT_DEF_CMD_DECLINT:
      case RT_DEF_CMD_DECLREAL:
      case RT_DEF_CMD_DECLSTRING:
      case RT_DEF_CMD_DECLPARAM:
      case RT_DEF_CMD_DECLREG:
      case RT_DEF_CMD_DECLTIME:
      case RT_DEF_CMD_DECLEVENT:
      case RT_DEF_CMD_SETINT:
      case RT_DEF_CMD_SETREAL:
      case RT_DEF_CMD_SETSTRING:
      case RT_DEF_CMD_SETWIRE:
      case RT_DEF_CMD_DELVAR:
         class = RT_VCD;
         break;
      case RT_DEF_CMD_SETSTATE:
      case RT_DEF_CMD_DECLTASK:
      case RT_DEF_CMD_DECLMUTEX:
      case RT_DEF_CMD_DECLOBJ:
      case RT_DEF_CMD_WAIT:
      case RT_DEF_CMD_SWITCH:
      case RT_DEF_CMD_READY:
      case RT_DEF_CMD_PREEMPT:
      case RT_DEF_CMD_CREATGRP:
      case RT_DEF_CMD_DELGRP:
      case RT_DEF_CMD_RUN:
      case RT_DEF_CMD_CREATTASK:
      case RT_DEF_CMD_CREATOBJ:
      case RT_DEF_CMD_CREATMUTEX:
      case RT_DEF_CMD_CALL:
      case RT_DEF_CMD_RETURN:
      case RT_DEF_CMD_DELMUTEX:
      case RT_DEF_CMD_DELTASK:
      case RT_DEF_CMD_DELOBJ:
      case RT_DEF_CMD_ACQUIRE:
      case RT_DEF_CMD_SETGLOBAL:
         class = RT_MSC|RT_VCD;
         break;
      case RT_DEF_CMD_COMMENT:
      case RT_DEF_CMD_SENDMSG:
      case RT_DEF_CMD_RECVMSG:
      case RT_DEF_CMD_ACTION:
      case RT_DEF_CMD_SETTIMER:
      case RT_DEF_CMD_TIMEOUT:
      case RT_DEF_CMD_STOPTIMER:
      case RT_DEF_CMD_TAKE:
      case RT_DEF_CMD_GIVE:
         class = RT_MSC;
         break;
   }

   return class;
}

/**
 * extract from a command what we need to do (new, check or del) on objects for id1 and i2
 */
void get_cmd_syntax(struct rt_msg * m,
                    int           * chk_group,
                    int           * chk_param1,
                    int           * chk_param2,
                    int           * new_param1,
                    int           * new_param2,
                    int           * del_param1,
                    int           * del_param2)
{
   *chk_param1 = RT_NONE;
   *chk_group  = RT_NONE;
   *chk_param2 = RT_NONE;
   *new_param1 = RT_NONE;
   *new_param2 = RT_NONE;
   *del_param1 = RT_NONE;
   *del_param2 = RT_NONE;

   switch(m->cmd)
   {
      case RT_DEF_CMD_DECLTASK:
         *new_param1 = RT_TASK;
         *chk_group  = RT_GROUP;
         break;
      case RT_DEF_CMD_DECLMUTEX:
         *new_param1 = RT_MUTEX;
         *chk_group  = RT_GROUP;
         break;
      case RT_DEF_CMD_DECLOBJ:
         *new_param1 = RT_OBJECT;
         *chk_group  = RT_GROUP;
         break;
      case RT_DEF_CMD_SENDMSG:
         *chk_param1 = RT_TASK;
         *chk_param2 = RT_TASK;
         break;
      case RT_DEF_CMD_CALL:
         *chk_param1 = RT_TASK|RT_OBJECT;
         *chk_param2 = RT_TASK|RT_OBJECT;
         break;
      case RT_DEF_CMD_RECVMSG:
         *chk_param1 = RT_TASK;
         *chk_param2 = RT_TASK;
         break;
      case RT_DEF_CMD_WAIT:
         *chk_param1 = RT_TASK;
         break;
      case RT_DEF_CMD_SWITCH:
         *chk_param1 = RT_TASK;
         *chk_param2 = RT_TASK;
         break;
      case RT_DEF_CMD_RETURN:
         *chk_param1 = RT_TASK|RT_OBJECT|RT_MUTEX;
         *chk_param2 = RT_TASK|RT_OBJECT;
         break;
      case RT_DEF_CMD_COMMENT:
         *chk_param1 = RT_TASK|RT_OBJECT|RT_MUTEX;
         break;
      case RT_DEF_CMD_ACTION:
         *chk_param1 = RT_TASK|RT_OBJECT|RT_MUTEX;
         break;
      case RT_DEF_CMD_SETTIMER:
         *chk_param1 = RT_TASK|RT_OBJECT|RT_MUTEX;
         break;
      case RT_DEF_CMD_TIMEOUT:
         *chk_param1 = RT_TASK|RT_OBJECT|RT_MUTEX;
         break;
      case RT_DEF_CMD_STOPTIMER:
         *chk_param1 = RT_TASK|RT_OBJECT|RT_MUTEX;
         break;
      case RT_DEF_CMD_READY:
         *chk_param1 = RT_TASK|RT_OBJECT|RT_MUTEX;
         break;
      case RT_DEF_CMD_PREEMPT:
         *chk_param1 = RT_TASK|RT_OBJECT|RT_MUTEX;
         break;
      case RT_DEF_CMD_CREATTASK:
         *chk_param1 = RT_TASK|RT_OBJECT;
         *new_param2 = RT_TASK;
         *chk_group  = RT_GROUP;
         break;
      case RT_DEF_CMD_CREATMUTEX:
         *chk_param1 = RT_TASK|RT_OBJECT;
         *new_param2 = RT_MUTEX;
         *chk_group  = RT_GROUP;
         break;
      case RT_DEF_CMD_CREATOBJ:
         *chk_param1 = RT_TASK|RT_OBJECT;
         *new_param2 = RT_OBJECT;
         *chk_group  = RT_GROUP;
         break;
      case RT_DEF_CMD_TAKE:
         *chk_param1 = RT_TASK|RT_OBJECT;
         *chk_param2 = RT_MUTEX;
         break;
      case RT_DEF_CMD_GIVE:
         *chk_param1 = RT_TASK|RT_OBJECT;
         *chk_param2 = RT_MUTEX;
         break;
      case RT_DEF_CMD_DELMUTEX:
         *chk_param1 = RT_TASK|RT_OBJECT;
         *del_param2 = RT_MUTEX;
         break;
      case RT_DEF_CMD_DELTASK:
         *chk_param1 = RT_TASK|RT_OBJECT;
         *del_param2 = RT_TASK;
         break;
      case RT_DEF_CMD_DELOBJ:
         *chk_param1 = RT_TASK|RT_OBJECT;
         *del_param2 = RT_OBJECT;
         break;
      case RT_DEF_CMD_DECLBOOL:
         *chk_group  = RT_GROUP;
         *new_param1 = RT_BOOL;
         break;
      case RT_DEF_CMD_DECLWIRE:
         *chk_group  = RT_GROUP;
         *new_param1 = RT_WIRE;
         break;
      case RT_DEF_CMD_DECLINT:
         *chk_group  = RT_GROUP;
         *new_param1 = RT_INT;
         break;
      case RT_DEF_CMD_DECLREAL:
         *new_param1 = RT_REAL;
         *chk_group  = RT_GROUP;
         break;
      case RT_DEF_CMD_DECLSTRING:
         *new_param1 = RT_STRING;
         *chk_group  = RT_GROUP;
         break;
      case RT_DEF_CMD_DECLPARAM:
         *new_param1 = RT_PARAM;
         *chk_group  = RT_GROUP;
         break;
      case RT_DEF_CMD_DECLREG:
         *new_param1 = RT_REG;
         *chk_group  = RT_GROUP;
         break;
      case RT_DEF_CMD_DECLTIME:
         *new_param1 = RT_TIME;
         *chk_group  = RT_GROUP;
         break;
      case RT_DEF_CMD_DECLEVENT:
         *chk_group  = RT_GROUP;
         *new_param1 = RT_EVENT;
         break;
      case RT_DEF_CMD_SETSTATE:
         *chk_param1 = RT_TASK|RT_OBJECT;
         break;
      case RT_DEF_CMD_SETINT:
         *chk_param1 = RT_INT;
         break;
      case RT_DEF_CMD_SETREAL:
         *chk_param1 = RT_REAL;
         break;
      case RT_DEF_CMD_SETREG:
         *chk_param1 = RT_REG;
         break;
      case RT_DEF_CMD_SETPARAM:
         *chk_param1 = RT_PARAM;
         break;
      case RT_DEF_CMD_SETBOOL:
         *chk_param1 = RT_BOOL;
         break;
      case RT_DEF_CMD_SETWIRE:
         *chk_param1 = RT_WIRE;
         break;
      case RT_DEF_CMD_SETEVENT:
         *chk_param1 = RT_EVENT;
         break;
      case RT_DEF_CMD_SETSTRING:
         *chk_param1 = RT_STRING;
         break;
      case RT_DEF_CMD_SETTIME:
         *chk_param1 = RT_TIME;
         break;
      case RT_DEF_CMD_CREATGRP:
         *chk_group  = RT_GROUP;
         *new_param1 = RT_GROUP;
         break;
      case RT_DEF_CMD_DELGRP:
         *del_param1 = RT_GROUP;
         break;
      case RT_DEF_CMD_RUN:
         *chk_param1 = RT_TASK|RT_OBJECT|RT_MUTEX;
         break;
      case RT_DEF_CMD_ACQUIRE:
         *chk_param1 = RT_MUTEX;
         *chk_param2 = RT_TASK|RT_OBJECT;
         break;
      case RT_DEF_CMD_DELVAR:
         *del_param1 = RT_REAL|RT_REG|RT_PARAM|RT_WIRE|RT_BOOL|RT_TIME|RT_EVENT|RT_STRING|RT_INT;
         break;
      case RT_DEF_CMD_SETGLOBAL:
         *chk_param1 = RT_TASK|RT_OBJECT|RT_MUTEX|RT_REAL|RT_REG|RT_PARAM|RT_WIRE|RT_BOOL|RT_TIME|RT_EVENT|RT_STRING|RT_INT;
         break;
   }

   /* objects that need to be deleted must be checked before exec_cmd */
   *chk_param1 |= *del_param1;
   *chk_param2 |= *del_param2;
}

/**
 * alloc needed objects depending on parameters
 */
int alloc_params(struct rt_msg * m, int new_param1, int new_param2)
{
   if(new_param1 != RT_NONE)
   {
      m->obj1 = add_object(m->fid, m->id1, new_param1, m->group, m->text);
      if (m->obj1 == NULL)
         return -1;
   }

   if(new_param2 != RT_NONE)
   {
      m->obj2 = add_object(m->fid, m->id2, new_param2, m->group, m->text);
      if (m->obj2 == NULL)
         return -1;
   }
   return 0;
}


/**
 * check object references depending on parameters
 */
int check_params(struct rt_msg * m, int chk_group, int chk_param1, int chk_param2)
{
   if(chk_group != RT_NONE) // group zero corresponds to the implicit 'top', so is accepted
   {
      m->group = find_object(m->fid, m->gid, 1); // allow seraching globally if locally not found
      if(m->group == NULL)
      {
         ERROR("Bad group reference : cmd '%s' at @%d\n", rt_cmd_name(m->cmd), m->time);
         return -1;
      }
      if((m->group->type & chk_group) != m->group->type)
      {
         ERROR("Bad group type : cmd '%s' at @%d as invalid type %s\n", rt_cmd_name(m->cmd), m->time, rt_type_name(m->group->type));
         print_msg(m);
         return -1;
      }
   }

   if(chk_param1 != RT_NONE)
   {
      m->obj1 = find_object(m->fid, m->id1, 1); // allow seraching globally if locally not found
      if(m->obj1 == NULL)
      {
         ERROR("Bad identifier1 reference %x : cmd '%s' at @%d\n", m->id1, rt_cmd_name(m->cmd), m->time);
         return -1;
      }
      if((m->obj1->type & chk_param1) != m->obj1->type)
      {
         ERROR("Bad identifier1 type : cmd '%s' at @%d as invalid type %s\n", rt_cmd_name(m->cmd), m->time, rt_type_name(m->obj1->type));
         print_msg(m);
         return -1;
      }
   }

   if(chk_param2 != RT_NONE)
   {
      m->obj2 = find_object(m->fid, m->id2, 1); // allow seraching globally if locally not found
      if(m->obj2 == NULL)
      {
         ERROR("Bad identifier2 reference %x : cmd '%s' at @%d\n", m->id2, rt_cmd_name(m->cmd), m->time);
         return -1;
      }
      if((m->obj2->type & chk_param2) != m->obj2->type)
      {
         ERROR("Bad identifier2 type : cmd '%s' at @%d as invalid type %s\n", rt_cmd_name(m->cmd), m->time, rt_type_name(m->obj2->type));
         return -1;
      }
   }
   return 0;
}


/**
 * remove objects depending on parameters
 * if object is not vcd, it can be removed immediately. For vcd files, we cannot remove the
 * objects until the vcd definition phasis is finished.
 */
int free_params(struct rt_msg * m, int del_param1, int del_param2)
{
   if(del_param1 != RT_NONE)
   {
      if(del_object_by_id(m->fid, m->id1, 1) < 0)
         return -1;
   }

   if(del_param2 != RT_NONE)
   {
      if(del_object_by_id(m->fid, m->id2, 1) < 0)
         return -1;
   }
   return 0;
}


/**
 * a new command as been sorted and is ready to be executed
 */
void exec_cmd(struct rt_msg * m)
{
   switch(m->cmd)
   {
      case RT_DEF_CMD_DECLTASK:
         exec_decltask(m);
         break;
      case RT_DEF_CMD_DECLMUTEX:
         exec_declmutex(m);
         break;
      case RT_DEF_CMD_DECLOBJ:
         exec_declobj(m);
         break;
      case RT_DEF_CMD_SENDMSG:
         exec_sendmsg(m);
         break;
      case RT_DEF_CMD_CALL:
         exec_call(m);
         break;
      case RT_DEF_CMD_RECVMSG:
         exec_recvmsg(m);
         break;
      case RT_DEF_CMD_WAIT:
         exec_sleep(m);
         break;
      case RT_DEF_CMD_SWITCH:
         exec_switch(m);
         break;
      case RT_DEF_CMD_RETURN:
         exec_return(m);
         break;
      case RT_DEF_CMD_COMMENT:
         exec_comment(m);
         break;
      case RT_DEF_CMD_ACTION:
         exec_action(m);
         break;
      case RT_DEF_CMD_SETTIMER:
         exec_settimer(m);
         break;
      case RT_DEF_CMD_TIMEOUT:
         exec_exptimer(m);
         break;
      case RT_DEF_CMD_STOPTIMER:
         exec_stoptimer(m);
         break;
      case RT_DEF_CMD_READY:
         exec_ready(m);
         break;
      case RT_DEF_CMD_PREEMPT:
         exec_suspend(m);
         break;
      case RT_DEF_CMD_CREATTASK:
         exec_creattask(m);
         break;
      case RT_DEF_CMD_CREATMUTEX:
         exec_creatmutex(m);
         break;
      case RT_DEF_CMD_CREATOBJ:
         exec_creatobj(m);
         break;
      case RT_DEF_CMD_TAKE:
         exec_take(m);
         break;
      case RT_DEF_CMD_GIVE:
         exec_give(m);
         break;
      case RT_DEF_CMD_DELMUTEX:
         exec_delmutex(m);
         break;
      case RT_DEF_CMD_DELTASK:
         exec_deltask(m);
         break;
      case RT_DEF_CMD_DELOBJ:
         exec_delobj(m);
         break;
      case RT_DEF_CMD_DECLBOOL:
         exec_declbool(m);
         break;
      case RT_DEF_CMD_DECLWIRE:
         exec_declwire(m);
         break;
      case RT_DEF_CMD_DECLINT:
         exec_declint(m);
         break;
      case RT_DEF_CMD_DECLREAL:
         exec_declreal(m);
         break;
      case RT_DEF_CMD_DECLSTRING:
         exec_declstring(m);
         break;
      case RT_DEF_CMD_SETINT:
         exec_setint(m);
         break;
      case RT_DEF_CMD_SETREAL:
         exec_setreal(m);
         break;
      case RT_DEF_CMD_SETSTRING:
         exec_setstring(m);
         break;
      case RT_DEF_CMD_SETSTATE:
         exec_setstate(m);
         break;
      case RT_DEF_CMD_CREATGRP:
         exec_creategrp(m);
         break;
      case RT_DEF_CMD_DELGRP:
         exec_delgrp(m);
         break;
      case RT_DEF_CMD_RUN:
         exec_activate(m);
         break;
      case RT_DEF_CMD_ACQUIRE:
         exec_acquire(m);
         break;
      case RT_DEF_CMD_DECLEVENT:
         exec_declevent(m);
         break;
      case RT_DEF_CMD_DECLTIME:
         exec_decltime(m);
         break;
      case RT_DEF_CMD_DECLPARAM:
         exec_declparam(m);
         break;
      case RT_DEF_CMD_DECLREG:
         exec_declreg(m);
         break;
      case RT_DEF_CMD_SETEVENT:
         exec_setevent(m);
         break;
      case RT_DEF_CMD_SETPARAM:
         exec_setparam(m);
         break;
      case RT_DEF_CMD_SETREG:
         exec_setreg(m);
         break;
      case RT_DEF_CMD_SETTIME:
         exec_settime(m);
         break;
      case RT_DEF_CMD_SETBOOL:
         exec_setbool(m);
         break;
      case RT_DEF_CMD_SETWIRE:
         exec_setwire(m);
         break;
      case RT_DEF_CMD_STARTDUMP:
         exec_startdump(m);
         break;
      case RT_DEF_CMD_STOPDUMP:
         exec_stopdump(m);
         break;
      case RT_DEF_CMD_DELVAR:
         exec_delvar(m);
         break;
      case RT_DEF_CMD_SETGLOBAL:
         exec_setglobal(m);
         break;

   }
}

/**
 * some stuf to do before executing any message
 * return 0 is the command can be exectued
 * return -1 else
 */
int process_cmd(struct rt_msg * m)
{
   int chk_group, chk_param1, chk_param2, new_param1, new_param2, del_param1, del_param2;
   rt_time_t off;

#if DEBUG(INFO)
   INFO_OPT(LOG_HAVE_NEXT                , "exe_cmd %-15s", rt_cmd_name(m->cmd));
   INFO_OPT(LOG_HAVE_NEXT | LOG_HAVE_PREV, " time %10d", m->time);
   INFO_OPT(LOG_HAVE_NEXT | LOG_HAVE_PREV, " fid  %8x", m->fid);
   INFO_OPT(LOG_HAVE_NEXT | LOG_HAVE_PREV, " gid  %8x", m->gid);
   INFO_OPT(LOG_HAVE_NEXT | LOG_HAVE_PREV, " id1  %8x", m->id1);
   INFO_OPT(LOG_HAVE_NEXT | LOG_HAVE_PREV, " id2  %8x", m->id2);
   INFO_OPT(                LOG_HAVE_PREV, " text '%s'\n", m->text);
#endif
   // msc check cmd
   if(m->class & RT_MSC)
   {
      // find a correlation for this message
      if(msc_out)
         msc_find_corr(m);

      /*
       * VERB("cmd_pre '%s' : time=%d off=%d msc_page=%d msc_page_max_levels=%d msc_level=%d\n",
       *      rt_cmd_name(m->cmd), msc_get_time(m), m->off, msc_page, msc_page_max_levels, msc_level);
       */

      // current level update
      if (msc_get_time(m) > msc_level)
      {
         if(msc_out)
         {
            int saved_vcd = vcd_out;
            int saved_sdl = sdl_out;

            // during a page break, disable write accesses to other files
            vcd_out = 0;
            sdl_out = 0;

            // break the page each time m->time() - msc_page > msc_page_max_levels
            while (msc_get_time(m) - msc_page >= msc_page_max_levels)
            {
               // compute nextlevel to terminate the page. current msc_level is supposed to be
               // inside a normal page
               off = msc_page_max_levels - (msc_level - msc_page);
               write_line(msc_fd, "\\nextlevel[%d]\n", off);
               msc_level += off;
               write_line(msc_fd, "%%level=%d\n", msc_level);

               // start a new page
               msc_page = msc_level;

               // start a new page
               msc_dump_stop(msc_fd, m->time);

               write_line(msc_fd, "\\newpage\n");
               msc_dump_start(msc_fd, "msc", m->time);
            }

            // restore file descriptors
            vcd_out = saved_vcd;
            sdl_out = saved_sdl;

            // update next level
            write_line(msc_fd, "\\nextlevel[%d]\n", msc_get_time(m) - msc_level);

         }

         // set new time
         msc_level = msc_get_time(m);

         if(msc_out)
         {
            // add a comment
            write_line(msc_fd, "%%level=%d\n", msc_level);
            if(msc_mark_grain == MSC_MARK_GRANULARITY_LEVEL) {
               switch(msc_mark_disp) {
                  case MSC_MARK_DISPLAY_NONE:
                     break;
                  case MSC_MARK_DISPLAY_BOTH:
                     write_line(msc_fd, "\\mscmark[bl]{%d : %d}{envleft}\n", m->time, msc_level);
                     break;
                  case MSC_MARK_DISPLAY_REALTIME:
                     write_line(msc_fd, "\\mscmark[bl]{%d}{envleft}\n", m->time);
                     break;
                  case MSC_MARK_DISPLAY_LEVEL:
                     write_line(msc_fd, "\\mscmark[bl]{%d}{envleft}\n", msc_level);
                     break;
               }
            }
         }
      }
      else if (msc_get_time(m) < msc_level)
      {
         ERROR("old message '%s' at @%d\n", rt_cmd_name(m->cmd), m->time);
         return -1;
      }

      // break correlation if a newpage is between
      if (msc_out && m->corr && (msc_get_time(m) + m->off - msc_page >= msc_page_max_levels))
      {
         m->corr->corr = NULL;
         m->corr       = NULL;
         VERB("break correlation\n");
      }
   }

   // vcd check cmd
   if(m->class & RT_VCD)
   {
      if (vcd_get_time(m) > vcd_level)
      {
         // set new time
         vcd_level = vcd_get_time(m);

         if ((vcd_level > 0) && vcd_out)
            write_line(vcd_fd, "#%d\n", vcd_level);
      }
      else if (vcd_get_time(m) < vcd_level)
      {
         ERROR("old message '%s' at @%d\n", rt_cmd_name(m->cmd), m->time);
         return -1;
      }
   }

   // see how the command behave with its objects
   get_cmd_syntax(m, &chk_group, &chk_param1, &chk_param2, &new_param1, &new_param2, &del_param1, &del_param2);

   // in vcd fifo mode all symbols must be declared at the beginning of the vcd dump file
   if((m->class & RT_VCD) && vcd_fifo)
   {
      int sym_def = (new_param1 | new_param2);

      // the value change dump section starts when time is > 0 or when detection something else than a symbol definition
      if(!vcd_def_end && ((vcd_get_time(m) > 0) || !sym_def))
      {
         vcd_write_definitions();
         vcd_def_end = 1;
      }
      else if(vcd_def_end && sym_def)
      {
         ERROR("vcd_fifo mode forbid declaration of symbols after definition phasis : cmd '%s' at @%d\n", rt_cmd_name(m->cmd), m->time);
         return -1;
      }
   }

   // check for correct parameter references, and link message to objects
   if(check_params(m, chk_group, chk_param1, chk_param2))
      return -1;

   // allocate new objects and link them to obj1 or obj2
   if(alloc_params(m, new_param1, new_param2))
      return -1;

   // execute the command
   exec_cmd(m);

   // free the objects that are no more necessary
   if(free_params(m, del_param1, del_param2))
      return -1;

   return 0;
}


/**
 * extract from the queue oldest messages, that are older than the newest ones, from rt_queue_flush distance.
 * In untimed mode, this function will first recompute all untimed levels for the whole queue
 */
void flush_queue()
{
   list_node_t * node, * tmp;
   struct rt_msg * m;

   rt_time_t end_time = list_entry(rt_queue.pprev, struct rt_msg, node)->time;
   rt_time_t start_time = list_entry(rt_queue.pnext, struct rt_msg, node)->time;

   if(start_time + rt_queue_flush <= end_time)
   {
      if (msc_untimed || vcd_untimed)
      {
         /* initialize previous parameters */
         rt_time_t msc_level = list_entry(rt_queue.pnext, struct rt_msg, node)->msc_level;
         rt_time_t vcd_level = list_entry(rt_queue.pnext, struct rt_msg, node)->vcd_level;
         rt_time_t rt_level = start_time;

         /* compute all untimed levels in untimed mode */
         list_for_each(node, &rt_queue)
         {
            m = list_entry(node, struct rt_msg, node);
            if(m->time > rt_level)
            {
               rt_level = m->time;
               if(m->class & RT_MSC)
                  msc_level += 1;
               if(m->class & RT_VCD)
                  vcd_level += 1;
            }
            m->msc_level = msc_level;
            m->vcd_level = vcd_level;
         }
      }

      /* execute all messages older than rt_flush_queue */
      list_for_each_safe(node, tmp, &rt_queue)
      {
         m = list_entry(node, struct rt_msg, node);
         if (m->time + rt_queue_flush <= end_time)
         {
            list_delete(&m->node);

            // process the command (check it, and execute it)
            process_cmd(m);

            // free the message
            heap_free(m);
         }
         else
         {
            break;
         }
      }
   }
}

/**
 * insert a message in the rt_queue so that the rt_queue is sorted by date, from the oldest one to the newest one.
 * if added on the top of the queue, all messages m' at head of the queue that verify m'->time < m->time + rt_queue_flush
 * are removed from the queue and processes.
 */
void add_msg(struct rt_msg * m)
{
   list_node_t * node;
   struct rt_msg * mbis;

   // find to which classes the message belong
   m->class = classify_cmd(m->cmd);

#if DEBUG(INFO)
   INFO_OPT(LOG_HAVE_NEXT                , "add_cmd %-15s", rt_cmd_name(m->cmd));
   INFO_OPT(LOG_HAVE_NEXT | LOG_HAVE_PREV, " time %10d", m->time);
   INFO_OPT(LOG_HAVE_NEXT | LOG_HAVE_PREV, " fid  %8x", m->fid);
   INFO_OPT(LOG_HAVE_NEXT | LOG_HAVE_PREV, " gid  %8x", m->gid);
   INFO_OPT(LOG_HAVE_NEXT | LOG_HAVE_PREV, " id1  %8x", m->id1);
   INFO_OPT(LOG_HAVE_NEXT | LOG_HAVE_PREV, " id2  %8x", m->id2);
   INFO_OPT(                LOG_HAVE_PREV, " text '%s'\n", m->text);
#endif

   if(list_empty(&rt_queue))
   {
      VERB("=> add_tail\n");
      list_add_tail(&m->node, &rt_queue);
   }
   /* retrive the top of the queue */
   else if (m->time >= list_entry(rt_queue.pprev, struct rt_msg, node)->time)
   {
      VERB("=> add_tail + flush queue\n");
      list_add_tail(&m->node, &rt_queue);

      flush_queue();
   }
   else if (m->time < list_entry(rt_queue.pnext, struct rt_msg, node)->time)
   {
      VERB("=> add_head + flush queue\n");
      list_add_head(&m->node, &rt_queue);

      flush_queue();
   }
   else
   {
      /* msg will be inserted in ascending order, starting from the newest ones (most probable) */
      list_for_each_rev(node, &rt_queue)
      {
         mbis = list_entry(node, struct rt_msg, node);
         if (m->time >= mbis->time)
         {
            VERB("=> add_middle\n");
            list_insert_after(&m->node, &mbis->node);
            break;
         }
      }
   }
}

/**
 * given a text command, parse it and at it to the rt_queue.
 * Then process the new rt_queue
 */
int read_binary_cmd(int fid, char * buffer, int len)
{
   struct rt_msg * m = (struct rt_msg *) heap_alloc(sizeof(struct rt_msg));
   int rc;

   if(!m)
   {
      ERROR("Cannot allocate one 'rt_msg'\n");
      return -1;
   }

#if DEBUG(VERB)
   VERB_OPT(LOG_HAVE_NEXT, "read_bin %d bytes\n", len);
   for(rc = 0; rc < len; rc++)
      VERB_OPT(LOG_HAVE_NEXT | LOG_HAVE_PREV, ":%02x", buffer[rc]);
   VERB_OPT(LOG_HAVE_PREV, "\n");
#endif

   m->off = 0;
   m->corr = NULL;
   m->msc_level = 0;
   m->vcd_level = 0;
   m->fid = fid;

   // the way we extract a command depends on the encoding (text or binary)
   rc = rt_msg_from_buf(buffer, len, &m->cmd, &m->time, &m->gid, &m->id1, &m->id2, m->text);

   if (rc == 0)
   {
      add_msg(m);
   }
   else
   {
      ERROR("Invalid binary cmd\n");
      return -1;
   }
   return 0;
}


/**
 * given a text command, parse it and at it to the rt_queue.
 * Then process the new rt_queue
 */
int read_text_cmd(int fid, char * buffer, int len)
{
   struct rt_msg * m = (struct rt_msg *) heap_alloc(sizeof(struct rt_msg));
   int rc;

   VERB("read_text : %s\n", buffer);
   if(!m)
   {
      ERROR("Cannot allocate one 'rt_msg'\n");
      return -1;
   }

   m->off = 0;
   m->corr = NULL;
   m->msc_level = 0;
   m->vcd_level = 0;
   m->fid = fid;

   // the way we extract a command depends on the encoding (text or binary)
   rc = rt_msg_from_string(buffer, &m->cmd, &m->time, &m->gid, &m->id1, &m->id2, m->text);
      
   if (rc == 0)
   {
      add_msg(m);
   }
   else if(buffer[0] == '#' || buffer[0] == '%' || buffer[0] == '\0')
   {
      // this is interpreted as a comment in the source file
   }
   else
   {
      ERROR("Invalid cmd : %s\n", buffer);
      return -1;
   }
   return 0;
}

void display_help()
{
   fprintf(stdout, "Syntax:\n");
   fprintf(stdout, "\t rtsv [options] -- <file1> <file2> ...\n\n");
   fprintf(stdout, "Description:\n");
   fprintf(stdout, "\t Start a real time server and generate waves or msc\n");
   fprintf(stdout, "\t If no file are provided, read data from stdin\n");
   fprintf(stdout, "\t .bin files are interpreted as binary format, otherwise it is rtsv format text\n\n");
   fprintf(stdout, "Options:\n");
   fprintf(stdout, "\t-freq <hz>             : (100000) frequency at which the rt_time clock is working\n");
   fprintf(stdout, "\t-queue <ticks>         : (1000) maximum rt_time_t between the oldest and newest msg in the queue\n");
   fprintf(stdout, "\t-vcd <file>            : output a vcd file\n");
   fprintf(stdout, "\t-msc <file>            : output a msc-latex file\n");
   fprintf(stdout, "\t-sdl <file>            : output a sdl dot file\n");
   fprintf(stdout, "\t-log <level>           : 0=NONE, 1=ASSERT, 2=ERROR, 3=INFO, 4=VERB\n");
   fprintf(stdout, "\t-msc_untimed           : increase time one by one for msc\n");
   fprintf(stdout, "\t-msc_out               : 0=OFF 1=ON(def). automatically start dumping at the beginning\n");
   fprintf(stdout, "\t-vcd_out               : 0=OFF 1=ON(def). automatically start dumping at the beginning\n");
   fprintf(stdout, "\t-msc_level_height <mm> : size in mm of one level\n");
   fprintf(stdout, "\t-msc_box_height   <mm> : size in mm of box inside one level\n");
   fprintf(stdout, "\t-msc_inst_dist    <mm> : size of a pdf page width, in mm\n");
   fprintf(stdout, "\t-msc_mark_grain   <m>  : mark granularity 0:none, 1:page, 2:level\n");
   fprintf(stdout, "\t-msc_mark_disp    <m>  : mark display 0:none, 1:real, 2:level, 3=both\n");
   fprintf(stdout, "\t-vcd_untimed           : increase time one by one for vcd\n");
}

int main(int argc, char ** argv)
{
   int nfds = 0;
   fd_set fds, rfds, bfds;
   int fd;
   int fdmax=0;
   char buffer[RT_CFG_MAX_COMMAND_LEN];
   int len;
   int i;

   char args[512];
   char * p;
   char * f;

   char msc_doc[RT_CFG_MAX_TEXT_LEN] = "";
   char sdl_doc[RT_CFG_MAX_TEXT_LEN] = "";
   char vcd_doc[RT_CFG_MAX_TEXT_LEN] = "";
   char title[RT_CFG_MAX_TEXT_LEN] = "";

   // clear descriptors
   FD_ZERO(&fds);
   FD_ZERO(&rfds);
   FD_ZERO(&bfds);

   // register a new log handler
   lib_set_log_handler(rtsv_log_handler, NULL);

   // init the queue of messages
   list_init(&rt_queue);

   // init top
   init_object(&top, "top", 0, 0, RT_GROUP, NULL, 0);
   top.global = 1;

   // options
   gopt_format(argc, argv, args, sizeof(args));

   if(gopt_find("-h", args, 512) || gopt_find("--help", args, 512))
   {
      display_help();
      return 0;
   }

   gopt_string  (title,               "-title", args, RT_CFG_MAX_TEXT_LEN);
   gopt_string  (vcd_doc,             "-vcd", args, RT_CFG_MAX_TEXT_LEN);
   gopt_string  (sdl_doc,             "-sdl", args, RT_CFG_MAX_TEXT_LEN);
   gopt_string  (msc_doc,             "-msc", args, RT_CFG_MAX_TEXT_LEN);
   gopt_integer(&rt_log_level,        "-log", args);
   gopt_bool   (&vcd_fifo,            "-vcd_fifo", args);
   gopt_bool   (&msc_untimed,         "-msc_untimed", args);
   gopt_bool   (&vcd_untimed,         "-vcd_untimed", args);
   gopt_long   (&rt_freq,             "-freq", args);
   gopt_integer(&rt_queue_flush,      "-queue", args);
   gopt_integer(&msc_inst_dist,       "-msc_inst_dist", args);
   gopt_integer(&msc_level_height,    "-msc_level_height", args);
   gopt_integer(&msc_box_height,      "-msc_box_height", args);
   gopt_integer(&msc_page_max_levels, "-msc_page_max_levels", args);
   gopt_integer(&msc_out,             "-msc_out", args);
   gopt_integer(&vcd_out,             "-vcd_out", args);

   gopt_integer((int*)&msc_mark_disp,  "-msc_mark_disp", args);
   gopt_integer((int*)&msc_mark_grain, "-msc_mark_grain", args);

   printf("msc_page_max_levels  = %d\n", msc_page_max_levels);
   printf("msc_level_height     = %d\n", msc_level_height);
   printf("msc_box_height       = %d\n", msc_box_height);
   printf("msc_inst_dist        = %d\n", msc_inst_dist);
   printf("msc_out              = %d\n", msc_out);

   // msc file
   if (string_len(msc_doc) > 0)
   {
      msc_fd = open("/tmp/msc_doc", O_CREAT|O_WRONLY|O_TRUNC, 0666);
      if(msc_fd < 0)
      {
         ERROR("Cannot open '/tmp/msc_doc' for write\n");
         return -1;
      }

      msc_new_doc(msc_fd);

      if(msc_out)
         msc_dump_start(msc_fd, "msc", 0);
   }

   // sdl file
   if (string_len(sdl_doc) > 0)
   {
      sdl_fd = open(sdl_doc, O_CREAT|O_WRONLY|O_TRUNC, 0666);
      if (sdl_fd < 0)
      {
         ERROR("Cannot open %s for write\n", sdl_doc);
         return -1;
      }
      sdl_out = 1;
   }

   // vcd file
   if (string_len(vcd_doc) > 0)
   {
      if(vcd_fifo == 1)
      {
         vcd_fd = open(vcd_doc, O_CREAT|O_WRONLY|O_TRUNC, 0666);
         if (vcd_fd < 0)
         {
            ERROR("Cannot open %s for write\n", vcd_doc);
            return -1;
         }

         vcd_def_fd = vcd_fd;
      }
      else
      {
         // two temporar files will be created for definitions and value changes. At the end of the simulation, the two files are 'cat' to vcd_doc
         vcd_def_fd = open("/tmp/def.vcd", O_CREAT | O_WRONLY|O_TRUNC, 0666);
         if (vcd_def_fd < 0)
         {
            ERROR("Cannot open '/tmp/def.vcd' for write\n");
            return -1;
         }
         vcd_fd = open("/tmp/sim.vcd", O_CREAT|O_WRONLY|O_TRUNC, 0666);
         if (vcd_fd < 0)
         {
            ERROR("Cannot open '/tmp/sim.vcd' for write\n");
            return -1;
         }
      }

      vcd_new_doc(vcd_def_fd, title);
   }

   // open input files for reading
   p = gopt_find("--", args, 512);
 
   if (p == NULL)
   {
      INFO("read from 'stdin'\n");
      fdmax = open("/dev/stdin", O_RDONLY, 0666);
      FD_SET(fdmax, &rfds);
      nfds = 1;
   }
   else
   {
      char * delim = " \r\t\n";
      string_sep(&p, delim);
      while(p)
      {
         // printf("p='%s'\n", p);
         f = string_sep(&p, delim);
         // printf("f='%s'\n", f);

         if(f && (string_len(f) > 0))
         {
            char * ext;
            int slen = string_len(f) - 1;

            // retreive filename extension, assuming .bin are in binary format, and others in text format
            while(slen > 0)
            {
               if(f[slen] == '.')
                  break;
               slen--;
            }

            if(slen > 0)
               ext = &f[slen+1];
            else
               ext = "";

            fd = open(f, O_RDWR);
            if (fd > 0)
            {
               INFO("'%s' opened, fd=%d ext='%s'\n", f, fd, ext);
               FD_SET(fd, &rfds);
               if (fd > fdmax)
                  fdmax = fd;

               if(string_cmp(ext, "bin") == 0)
                  FD_SET(fd, &bfds);

               nfds++;
            }
            else
            {
               ERROR("-E cannot open '%s' : ret=%d\n", f, fd);
            }
         }
      }
   }

   // process all input files at the same time open input files for reading
   while (nfds > 0)
   {
      // reload file descriptors to read
      mem_cpy(&fds, &rfds, sizeof(fds));

      int nfdsr = select(fdmax + 1, &fds, NULL, NULL, NULL);
      for (fd = 0; fd <= fdmax; fd++)
      {
         if (FD_ISSET(fd, &fds))
         {
            int binary = FD_ISSET(fd, &bfds);

            // read a line in text mode or a block of data in binary mode
            if(binary)
               len = read_data(fd, buffer, RT_CFG_MAX_COMMAND_LEN);
            else
               len = read_line(fd, buffer, RT_CFG_MAX_COMMAND_LEN);

            if(len <= 0)
            {
               // remove descriptor from the list
               FD_CLR(fd, &rfds);
               INFO("fd %d end of file\n", fd);
               nfds--;
            }
            else if(binary)
            {
               read_binary_cmd(fd, buffer, len);
            }
            else
            {
               read_text_cmd(fd, buffer, len);
            }
         }
      }
   }
   // flush last queued messages
   rt_queue_flush = 0;
   flush_queue();

   // close opened files
   if(msc_fd > 0)
   {
      char cmd[512];
      char basename[126];
      int slen = string_len(msc_doc) - 1;
      while(slen > 0)
      {
         if (msc_doc[slen] == '.')
            break;
         slen--;
      }
      if(slen > 0)
      {
         string_ncpy(basename, msc_doc, slen);
         basename[slen] = 0;
      }
      else
      {
         string_cpy(basename, msc_doc);
      }

      if(msc_out)
         msc_dump_stop(msc_fd, msc_level);

      msc_end_doc(msc_fd);
      INFO("Found a maximum of %d instances in the whole document\n", msc_max_instances);

      // compute the maximum number of levels per page
      int msc_page_height = (msc_page_max_levels + 7) * msc_level_height;
      int msc_page_width =  (msc_max_instances + 2 /*env left + env right */ - 1) * msc_inst_dist + 20 /* left + right margin */;

      // replace PAPERWIDTH and PAPERHEIGHT by the adequate value
      string_printf(cmd, "sed 's/PAPERWIDTH/%d/g;s/PAPERHEIGHT/%d/g' /tmp/msc_doc > %s",
                    msc_page_width, msc_page_height, msc_doc);
      printf("%s\n", cmd);
      system(cmd);
      close(msc_fd);

      INFO("make pdf latex %s.pdf ... \n", basename);
      string_printf(cmd, "latex %s > /tmp/log", msc_doc);
      printf("%s\n", cmd);
      system(cmd);

      string_printf(cmd, "dvipdf %s.dvi > /dev/null", basename);
      printf("%s\n", cmd);
      system(cmd);
      INFO(".. done!\n", basename);
   }

   if(sdl_fd > 0)
      close(sdl_fd);

   if(vcd_fd > 0)
   {
      close(vcd_fd);

      if (vcd_fifo == 0)
      {
         char cmd[512];

         // redraw instances
         vcd_write_definitions();

         close(vcd_def_fd);
         INFO("concatenante def.vcd and sim.vcd\n");
         string_printf(cmd, "cat /tmp/def.vcd /tmp/sim.vcd > %s\n", vcd_doc);
         system(cmd);
      }
   }

   // remove memory
   for_each_object(&top, remove_iterator, NULL);

   return 0;
}
