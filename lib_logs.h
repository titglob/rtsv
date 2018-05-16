/*!
 * \file lib_logs.h
 *
 * The objective of this file is to provide a generic log interface that
 * can format a text by adding
 * - a module name
 * - a severity
 * - a timestamp
 * - color
 *
 * and outputing data to various king of outputs (uart, file system)
 *
 * Moreover, this library must be able to tabulate all the text if required by the called
 * 
 * This is not thread safe nor protected agains interruptions if called from this context.
 *
 * The best idea is to implement a low level task responsible of outputing logs...
 * In case of non os mode, the called need to lock the interrupts
 *
 * Note for VFS: the low level functions fs_read/fs_write/fs_open... are unresolved symbols
 * for LEON.
 *
 * One idea is to declare a log_handle pointeur that is init at log_init() time and
 * can be redefined later to be better handler
 */

#ifndef LIB_LOGS
#define LIB_LOGS

#include <color.h>
#include <cpu.h>

/** 
 * \addtogroup PAL
 * @{
 * \addtogroup LIB
 * @{
 * \addtogroup logs
 * @{
 */

/**
 * log uart and custom logfile fd
 * PRINT: use dbg_print(dbg_stderr, ... for ASSERT and ERROR logs
 *        use dbg_print(dbg_stdout, ... for other logs
 * MBX: for embedded only: pass througth mailbox to host
 * USER: not handled by sdk, but can be displayed by the user if the log_handler_ext
 *       is overwritten.
 */
#define LOG_DEF_PRINT -1
#define LOG_DEF_MBX  -2
#define LOG_DEF_USER  -3

/**
 * definitions used for access logging attributes
 */
#define LOG_DEF_ATTR_READ      (1 << 0)
#define LOG_DEF_ATTR_WRITE     (1 << 1)
#define LOG_DEF_ATTR_READ_DATA (1 << 2)                 // read data is not supposed to be valid if this flag is not set
#define LOG_DEF_ATTR_BIT_8     (1 << 3)
#define LOG_DEF_ATTR_BIT_16    (1 << 4)
#define LOG_DEF_ATTR_BIT_32    (1 << 5)

/**
 * log table state.
 * Any other value indicates that either the LOG_CFG_DEBUG_BUS_ENABLED is not activated in the firmware,
 * or that the table is corrupted so not readable.
 */
#define LOG_DEF_DEBUG_BUS_RUNNING  0xcafebabe

/**
 * log level values
 */
#define DEBUG_NONE     (0)
#define DEBUG_ASSERT   (1)
#define DEBUG_ERROR    (2)
#define DEBUG_INFO     (3)
#define DEBUG_VERB     (4)

/**
 *  log output
 */

/**
 * log option
 */
#define LOG_TIME          (1 << 0)  //!< add timing info to output
#define LOG_HAVE_PREV     (1 << 1)  //!< indicates that the log follow a preivous one atomically
#define LOG_HAVE_NEXT     (1 << 2)  //!< indicates that the log will be followed atomically by another one
#define LOG_IRQ_SAFE      (1 << 3)  //!< protect output from interrupts (in case log are called also in isr)
#define LOG_THREAD_SAFE   (1 << 4)  //!< protect output from context switch

/**
 * each transfer is registered into one struct like that
 */
struct log_entry
{
   uint32_t addr;
   uint32_t data;
   uint32_t attr;
   uint32_t time;
};

/**
 * This table is mapped into the firmware memory and
 * collect information on the last embedded transfers.
 */
struct log_table
{
   uint32_t state;  //!< matches LOG_DEF_DEBUG_BUS_RUNNING when activated. Other values means that the feature is not enabled in the leon
   uint32_t count;  //!< number of bus accesses made by the firmware
   uint32_t index;  //!< next available index. Between [0 : max-1] 
   uint32_t max;    //!< maximum number of recordable accesses.
};


#ifdef __cplusplus
extern "C" {
#endif

typedef void (*log_handler_t)(int output, char * text);
typedef void (*log_handler_ext_t)(int severity, int output, char * text);

/*!
 * By default, log_output_ext_def function is set a lib_log_init time to output traces.
 * It can be overloader later to a better or high level function
 * For compat reasons, if the olf=d handler log_handler is defined, it replace the behaviour
 * of the new function, until 9.0.4 version.
 * A good practise when overloading this handler is to save the old handler to call it as
 * a default behaviour. For example, the usr handler can handle every logs for 'output' USER,
 * and call the pre-installed one (log_output_ext_def) for the other cases.
 */

/**
 * log api
 */

int     lib_log_init();
int     lib_log_end();

void    log_tab_inc();
void    log_tab_dec();

void    log_print(const char * module, int options, int severity, const char * color, int output, const char * fmt, ...);

#define log_print_def(fmt, ...)     log_print(NULL, 0, DEBUG_INFO, NULL, LOG_DEF_PRINT, fmt, ##__VA_ARGS__)

void    log_output_def(int output, char * text);
void    log_output_ext_def(int severity, int output, char * text);

void    log_access(uint32_t addr, uint32_t data, uint32_t attr);
void    log_print_entry(struct log_entry * le, int n);

/**
 * \brief register a new log handler
 * @param[in] new_handler the new handler to register
 * @param[out] old_handler. If not NULL, will store the old handler
 * @return 0 if success, < 0 else
 */
int     lib_set_log_handler(log_handler_ext_t new_handler, log_handler_ext_t * old_handler);

/*
 * @} logs
 * @} LIB
 * @} PAL
 */

#ifdef __cplusplus
}
#endif

#endif
