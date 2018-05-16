#ifndef LIB_RT_H
#define LIB_RT_H

#include <cpu.h>

#include <time.h>

#define RT_CFG_RTCLI_EN 1
#define RT_CFG_RTSV_EN  1

#ifdef __cplusplus
extern "C"
{
#endif

/*------------------------------------------------------------------------------
 * Public types
 *----------------------------------------------------------------------------*/

typedef uint32_t rt_time_t;    /// system time type

/**
 * object_id size depends on processor data bus width
 */
#define RT_OBJECT_SIZE         (CPU_DATA_SIZE / 8)

#if RT_OBJECT_SIZE == 8
typedef uint64_t object_id_t;  /// identifier for any object
#else
typedef uint32_t object_id_t;  /// identifier for any object
#endif

/**
 * list of supported commands
 */
typedef enum rt_cmd
{
   RT_DEF_CMD_DECLTASK        ,
   RT_DEF_CMD_DECLMUTEX       ,
   RT_DEF_CMD_DECLOBJ         ,
   RT_DEF_CMD_SENDMSG         ,
   RT_DEF_CMD_CALL            ,
   RT_DEF_CMD_RECVMSG         ,
   RT_DEF_CMD_SWITCH          ,
   RT_DEF_CMD_RETURN          ,
   RT_DEF_CMD_COMMENT         ,
   RT_DEF_CMD_ACTION          ,
   RT_DEF_CMD_SETTIMER        ,
   RT_DEF_CMD_TIMEOUT         ,
   RT_DEF_CMD_STOPTIMER       ,
   RT_DEF_CMD_CREATTASK       ,
   RT_DEF_CMD_CREATMUTEX      ,
   RT_DEF_CMD_CREATOBJ        ,
   RT_DEF_CMD_TAKE            ,
   RT_DEF_CMD_GIVE            ,
   RT_DEF_CMD_DELMUTEX        ,
   RT_DEF_CMD_DELTASK         ,
   RT_DEF_CMD_DELOBJ          ,
   RT_DEF_CMD_DECLBOOL        ,
   RT_DEF_CMD_DECLWIRE        ,
   RT_DEF_CMD_DECLINT         ,
   RT_DEF_CMD_DECLREAL        ,
   RT_DEF_CMD_DECLSTRING      ,
   RT_DEF_CMD_SETINT          ,
   RT_DEF_CMD_SETREAL         ,
   RT_DEF_CMD_SETSTRING       ,
   RT_DEF_CMD_SETSTATE        ,
   RT_DEF_CMD_CREATGRP        ,
   RT_DEF_CMD_DELGRP          ,
   RT_DEF_CMD_ACQUIRE         ,
   RT_DEF_CMD_DECLEVENT       ,
   RT_DEF_CMD_DECLTIME        ,
   RT_DEF_CMD_DECLPARAM       ,
   RT_DEF_CMD_DECLREG         ,
   RT_DEF_CMD_SETEVENT        ,
   RT_DEF_CMD_SETPARAM        ,
   RT_DEF_CMD_SETREG          ,
   RT_DEF_CMD_SETTIME         ,
   RT_DEF_CMD_SETBOOL         ,
   RT_DEF_CMD_SETWIRE         ,
   RT_DEF_CMD_STARTDUMP       ,
   RT_DEF_CMD_STOPDUMP        ,
   RT_DEF_CMD_DELVAR          ,
   RT_DEF_CMD_SETGLOBAL       ,

   RT_DEF_CMD_READY           ,
   RT_DEF_CMD_RUN             ,
   RT_DEF_CMD_PREEMPT         ,
   RT_DEF_CMD_IDLE            ,
   RT_DEF_CMD_WAIT            ,

   RT_DEF_CMD_MAX
}
rt_cmd_t;

/*------------------------------------------------------------------------------
 * Public definitions
 *----------------------------------------------------------------------------*/

/**
 * Origin for start of simulation
 */
#define RT_ORIG        -1

/**
 * Set current local time automatically
 */
#define RT_CUR          0

/**
 * Last current time
 */
#define RT_LAST        -2

/**
 * Root group
 */
#define RT_ROOT_GRP     0

/**
 * Root.Pal group
 */
#define RT_PAL_GRP      1

/**
 * Root.Pal.Os group
 */
#define RT_OS_GRP       2

/**
 * Initial bootstrapping task
 */
#define RT_OS_BOOT_TASK 3

/*------------------------------------------------------------------------------
 * Public programming wrappers
 *----------------------------------------------------------------------------*/

/**
 * force simulation to start
 */
#define rt_start_dump(time) \
   rt_log(time, RT_DEF_CMD_STARTDUMP, 0, 0, 0, "")

/**
 * force simulation to stop
 */
#define rt_stop_dump(time) \
   rt_log(time, RT_DEF_CMD_STOPDUMP, 0, 0, 0, "")

/**
 * Declare a statically created task (or thread).
 */
#define rt_decl_task(time, grp, id, name)                                      \
   rt_log(time, RT_DEF_CMD_DECLTASK, (object_id_t)(grp), (object_id_t)(id), 0, \
          name)

/**
 * Declare an interger number
 */
#define rt_decl_int(time, grp, id, name)                                       \
   rt_log(time, RT_DEF_CMD_DECLINT, (object_id_t)(grp), (object_id_t)(id), 0,  \
          name)


/**
 * Declare a real number
 */
#define rt_decl_real(time, grp, id, name)                                      \
   rt_log(time, RT_DEF_CMD_DECLREAL, (object_id_t)(grp), (object_id_t)(id), 0, \
          name)

/**
 * Declare a register
 */
#define rt_decl_reg(time, grp, id, name, sz)                                   \
   rt_log(time, RT_DEF_CMD_DECLREG, (object_id_t)(grp), (object_id_t)(id),     \
          (object_id_t)(sz), name)

/**
 * Declare a parameter
 */
#define rt_decl_param(time, grp, id, name, sz)                                 \
   rt_log(time, RT_DEF_CMD_DECLPARAM, (object_id_t)(grp), (object_id_t)(id),   \
          (object_id_t)(sz), name)

/**
 * Declare a string
 */
#define rt_decl_string(time, grp, id, name)                                    \
   rt_log(time, RT_DEF_CMD_DECLSTRING, (object_id_t)(grp), (object_id_t)(id),  \
          0, name)

/**
 * Declare a boolean
 */
#define rt_decl_bool(time, grp, id, name)                                      \
   rt_log(time, RT_DEF_CMD_DECLBOOL, (object_id_t)(grp), (object_id_t)(id), 0, \
          name)

/**
 * Declare a wire
 */
#define rt_decl_wire(time, grp, id, name, sz)                                  \
   rt_log(time, RT_DEF_CMD_DECLWIRE, (object_id_t)(grp), (object_id_t)(id),    \
          (object_id_t)(sz), name)

/**
 * Declare an event
 */
#define rt_decl_event(time, grp, id, name)                                     \
   rt_log(time, RT_DEF_CMD_DECLEVENT, (object_id_t)(grp), (object_id_t)(id),   \
          0, name)

/**
 * Declare a time object
 */
#define rt_decl_time(time, grp, id, name, sz)                                  \
   rt_log(time, RT_DEF_CMD_DECLTIME, (object_id_t)(grp), (object_id_t)(id),    \
          (object_id_t)(sz), name)

/**
 * Mark a task or object as eligible for running
 */
#define rt_ready(time, id) \
   rt_log(time, RT_DEF_CMD_READY, 0, (object_id_t)(id), 0, "")

/**
 * Mark a task or object as running
 */
#define rt_run(time, id) \
   rt_log(time, RT_DEF_CMD_RUN, 0, (object_id_t)(id), 0, "")

/**
 * Mark a task or object as preempted
 */
#define rt_preempt(time, id) \
   rt_log(time, RT_DEF_CMD_PREEMPT, 0, (object_id_t)(id), 0, "")

/**
 * Mark a task or object as idle
 */
#define rt_idle(time, id) \
   rt_log(time, RT_DEF_CMD_IDLE, 0, (object_id_t)(id), 0, "")

/**
 * Mark a task or object as waiting on resources
 */
#define rt_wait(time, id) \
   rt_log(time, RT_DEF_CMD_WAIT, 0, (object_id_t)(id), 0, "")

/**
 * Change the state of an object, task, mutex. Not applicable to variables
 */
#define rt_set_state(time, id, value) \
   rt_log(time, RT_DEF_CMD_SETSTATE, 0, (object_id_t)(id), (object_id_t)(value), "")

#define rt_set_state2(time, id, value) \
   rt_log(time, RT_DEF_CMD_SETSTATE, 0, (object_id_t)(id), 0, value)

/**
 * Change the value for an int
 */
#define rt_set_int(time, id, value)                                            \
   ({                                                                          \
      if(*(id) != value)                                                       \
         rt_log(time, RT_DEF_CMD_SETINT, 0, (object_id_t)(id),                 \
                (object_id_t)value, "");                                       \
    })


/**
 * Change the value for a real
 */
#define rt_set_real(time, id, value)                                           \
   ({                                                                          \
      if(*(id) != value)                                                       \
         rt_log(time, RT_DEF_CMD_SETREAL, 0, (object_id_t)(id),                \
                (object_id_t)value, "");                                       \
    })

/**
 * Change the value for a register
 */
#define rt_set_reg(time, id, value)                                            \
   ({                                                                          \
      if(*(id) != value)                                                       \
         rt_log(time, RT_DEF_CMD_SETREG, 0, (object_id_t)(id),                 \
                (object_id_t)value, "");                                       \
    })

/**
 * Change the value for a parameter
 */
#define rt_set_param(time, id, value)                                          \
   ({                                                                          \
      if(*(id) != value)                                                       \
         rt_log(time, RT_DEF_CMD_SETPARAM, 0, (object_id_t)(id),               \
                (object_id_t)value, "");                                       \
    })

/**
 * Change the value for a string
 */
#define rt_set_string(time, id, value) \
   rt_log(time, RT_DEF_CMD_SETSTRING, 0, (object_id_t)(id), 0, value)

/**
 * Change the value for a boolean
 */
#define rt_set_bool(time, id, value)                                           \
   ({                                                                          \
      if(*(id) != value)                                                       \
         rt_log(time, RT_DEF_CMD_SETBOOL, 0, (object_id_t)(id),                \
                (object_id_t)value, "");                                       \
    })

/**
 * Change the value for a wire
 */
#define rt_set_wire(time, id, value)                                           \
   ({                                                                          \
      if(*(id) != value)                                                       \
         rt_log(time, RT_DEF_CMD_SETWIRE, 0, (object_id_t)(id),                \
                (object_id_t)value, "");                                       \
    })

/**
 * Change the value for an event
 */
#define rt_set_event(time, id, value)                                          \
   ({                                                                          \
      if(*(id) != value)                                                       \
         rt_log(time, RT_DEF_CMD_SETEVENT, 0, (object_id_t)(id),               \
                (object_id_t)value, "");                                       \
    })

/**
 * Change the value for a register
 */
#define rt_set_time(time, id, value)                                           \
   ({                                                                          \
      if(*(id) != value)                                                       \
         rt_log(time, RT_DEF_CMD_SETTIME, 0, (object_id_t)(id),                \
                (object_id_t)value, "");                                       \
    })

/**
 * Notify message transmission
 */
#define rt_send_msg(time, id1, id2, inf)                                       \
      rt_log(time, RT_DEF_CMD_SENDMSG, 0, (object_id_t)(id1),                  \
             (object_id_t)(id2), inf)

/**
 * Notify message reception
 */
#define rt_recv_msg(time, id1, id2, inf)                                       \
      rt_log(time, RT_DEF_CMD_RECVMSG, 0, (object_id_t)(id1),                  \
             (object_id_t)(id2), inf)

/**
 * Add a comment
 */
#define rt_comment(time, id, text) \
   rt_log(time, RT_DEF_CMD_COMMENT, 0, (object_id_t)(id), 0, text)

/**
 * Start a timer
 */
#define rt_set_timer(time, id, text) \
   rt_log(time, RT_DEF_CMD_SETTIMER, 0, (object_id_t)(id), 0, text)

/**
 * Add an action
 */
#define rt_action(time, id, text) \
   rt_log(time, RT_DEF_CMD_ACTION, 0, (object_id_t)(id), 0, text)

/**
 * Timer timeout
 */
#define rt_timeout(time, id, text) \
   rt_log(time, RT_DEF_CMD_TIMEOUT, 0, (object_id_t)(id), 0, text)

/**
 * stop a timer
 */
#define rt_stop_timer(time, id, text) \
   rt_log(time, RT_DEF_CMD_STOPTIMER, 0, (object_id_t)(id), 0, text)

/**
 * create a task
 */
#define rt_create_task(time, grp, id1, id2, text)                              \
   rt_log(time, RT_DEF_CMD_CREATTASK, (object_id_t)(grp), (object_id_t)(id1),  \
          (object_id_t)(id2), text)

/**
 * delete a task
 */
#define rt_del_task(time, id1, id2)                                            \
   rt_log(time, RT_DEF_CMD_DELTASK, (object_id_t)0, (object_id_t)(id1),        \
          (object_id_t)(id2), "")

/**
 * create a mutex
 */
#define rt_create_mutex(time, grp, id1, id2, text)                             \
   rt_log(time, RT_DEF_CMD_CREATMUTEX, (object_id_t)(grp), (object_id_t)(id1), \
          (object_id_t)(id2), text)

/**
 * delete a mutex
 */
#define rt_del_mutex(time, id1, id2)                                           \
   rt_log(time, RT_DEF_CMD_DELMUTEX, (object_id_t)0, (object_id_t)(id1),       \
          (object_id_t)(id2), "")

/**
 * take a mutex
 */
#define rt_take(time, id1, id2) \
   rt_log(time, RT_DEF_CMD_TAKE, 0, (object_id_t)(id1), (object_id_t)(id2), "")

/**
 * acquire a mutex
 */
#define rt_acquire(time, id1, id2)                                             \
   rt_log(time, RT_DEF_CMD_ACQUIRE, 0, (object_id_t)(id1), (object_id_t)(id2), \
          "")

/**
 * give a mutex
 */
#define rt_give(time, id1, id2) \
   rt_log(time, RT_DEF_CMD_GIVE, 0, (object_id_t)(id1), (object_id_t)(id2), "")

/**
 * create an object
 */
#define rt_create_object(time, grp, id1, id2, text)                            \
   rt_log(time, RT_DEF_CMD_CREATOBJ, (object_id_t)(grp), (object_id_t)(id1),   \
          (object_id_t)(id2), text)

/**
 * call a object
 */
#define rt_call(time, id1, id2, text)                                          \
   rt_log(time, RT_DEF_CMD_CALL, 0, (object_id_t)(id1), (object_id_t)(id2),    \
          text)

/**
 * return from a call
 */
#define rt_return(time, id1, id2)                                              \
   rt_log(time, RT_DEF_CMD_RETURN, 0, (object_id_t)(id1), (object_id_t)(id2),  \
          "")

/**
 * delete an object
 */
#define rt_del_object(time, id1, id2)                                          \
   rt_log(time, RT_DEF_CMD_DELOBJ, 0, (object_id_t)(id1), (object_id_t)(id2),  \
          "")

/**
 * create a group
 */
#define rt_create_group(time, grp, id, name)                                   \
   rt_log(time, RT_DEF_CMD_CREATGRP, (object_id_t)(grp), (object_id_t)(id),    \
          (object_id_t)0, name)

/**
 * delete a group
 */
#define rt_del_group(time, id) \
   rt_log(time, RT_DEF_CMD_DELGRP, 0, (object_id_t)(id), (object_id_t)0, name)

/**
 * make object visibility to global by attributing it a global identifier
 */
#define rt_set_global(time, id, global_id)                                     \
   rt_log(time, RT_DEF_CMD_SETGLOBAL, 0, (object_id_t)(id),                    \
          (object_id_t)global_id, "")

/*------------------------------------------------------------------------------
 * Common interface
 *----------------------------------------------------------------------------*/

#if (RT_CFG_RTCLI_EN == 1) || (RT_CFG_RTSV_EN == 1)

/**
 * Maximum length for strings
 */
#define RT_CFG_MAX_TEXT_LEN    100

/**
 * Maximum length of a command in a text script
 */
#define RT_CFG_MAX_COMMAND_LEN 150

/**
 * Maximum grouping depth
 */
#define RT_CFG_MAX_HIERARCHY   32

/**
 * Number of bytes needed to store the size parameter
 */
#define RT_CFG_SIZE_LENGTH     1

/**
 * circular buffer parameters
 */
struct rt_trace_buffer
{
   uint32_t  rdptr; // must be the first field of the structure
   uint32_t  wrptr;
   uint32_t  start;
   uint32_t  size;
   uint32_t  end;
   uint32_t  errov;
   uint32_t  errsize;
};

/**
 * Return the system time
 */
rt_time_t rt_time(void);

/**
 * Synchronize the system time and return a time value
 */
rt_time_t rt_sync(rt_time_t external_time);

#else  /* !((RT_CFG_RTCLI_EN == 1) || (RT_CFG_RTSV_EN == 1)) */

static inline rt_time_t rt_time(void)
{
   return 0;
}

static inline rt_time_t rt_sync(rt_time_t external_time)
{
   return 0;
}

#endif /* !((RT_CFG_RTCLI_EN == 1) || (RT_CFG_RTSV_EN == 1)) */

/*----------------------------------------------------------------------------
 * Server side interface
 *----------------------------------------------------------------------------*/

#if (RT_CFG_RTSV_EN == 1)

/* Return cmd name */
const char * rt_cmd_name(rt_cmd_t cmd);

/**
 * Extract from a buffer of max length rt_msg arguments
 * Return 0 if no error, or < 0 if an error occured
 */
int rt_msg_from_buf(char * buf, int len, rt_cmd_t * cmd, rt_time_t * time,
                    object_id_t * grp, object_id_t * id1, object_id_t * id2,
                    char * text);

/**
 * Extract from a null terminated string correspoding to a rt_command the rt_msg
 * arguments * Return the number of bytes read from the line or < 0 if an error
 * occured
 */
int rt_msg_from_string(char * string, rt_cmd_t * cmd, rt_time_t * time,
                       object_id_t * grp, object_id_t * id1, object_id_t * id2,
                       char * text);

#endif /* RT_CFG_RTSV_EN == 1 */

/*----------------------------------------------------------------------------
 * Client side interface
 *----------------------------------------------------------------------------*/

#if RT_CFG_RTCLI_EN == 1

/**
 * Initialization of rt library
 */
int rt_init(const char **);

/**
 * low level method to output a rt_message to the host
 */
void rt_log(rt_time_t time, rt_cmd_t cmd, object_id_t grp, object_id_t id1,
            object_id_t id2, const char * name);

/**
 * low level method to output a text or binary command to the end user server
 */
int rt_output(const char * buffer, size_t len);

/**
 * End of rt library
 */
void rt_end();

#else /* !(RT_CFG_RTCLI_EN == 1) */

static inline int rt_init(const char ** )
{
   return 0;
}

static inline void rt_end(void)
{
}

static inline void rt_log(rt_time_t time, rt_cmd_t cmd, object_id_t grp,
                          object_id_t id1, object_id_t id2, const char * name)
{
}

#endif /* !(RT_CFG_RTCLI_EN == 1) */

#ifdef __cplusplus
}
#endif

#endif
