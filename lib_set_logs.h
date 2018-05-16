/**
 * this file can be included after <lib_logs.h> to define a default of specific module behaviour
 * for the current source file. Logs can be cleared by including <log_clear_logs.h> and set logs can be called
 * again with a new configuration
 */

#define CAT(DEFINE, ...)           DEFINE ## __VA_ARGS__

/**
 * default module is DEF
 */
#ifndef LOG_MODULE
#define LOG_MODULE      _DEF_
#endif

#ifndef _DEF__LOG_NAME
#define _DEF__LOG_NAME  NULL
#endif

#ifndef _DEF__LOG_LEVEL
#define _DEF__LOG_LEVEL   DEBUG_ASSERT
#endif

#ifndef _DEF__LOG_COLOR
#define _DEF__LOG_COLOR   NULL
#endif

#ifndef _DEF__LOG_OUTPUT
#define _DEF__LOG_OUTPUT   LOG_DEF_PRINT
#endif
/**
 * current module
 */
#define LOG_NAME(MOD)              CAT(MOD, _LOG_NAME)
#define LOG_COLOR(MOD)             CAT(MOD, _LOG_COLOR)
#define LOG_LEVEL(MOD)             CAT(MOD, _LOG_LEVEL)
#define LOG_OUTPUT(MOD)            CAT(MOD, _LOG_OUTPUT)

/**
 * usefull macro to use in code if a log level is set
 */
#define DEBUG(SEVERITY)             LOG_LEVEL(LOG_MODULE) >= DEBUG_##SEVERITY
#define DEBUG_MOD(MODULE, SEVERITY) LOG_LEVEL(_##MODULE##_) >= DEBUG_##SEVERITY

/**
 * assert api
 */
#if DEBUG(ASSERT)
#define ASSERT(expr)               if(!(expr)) {                                                                      \
                                        log_print(LOG_NAME(LOG_MODULE), 0, DEBUG_ASSERT, COLOR_RED, LOG_OUTPUT(LOG_MODULE),                  \
                                                  "ASSERT: " __FILE__ ":%d, " STRINGIFY(expr) "\n", __LINE__);        \
                                   }
#else
#define ASSERT(...)                do { } while(0)
#endif

/**
 * verbose api
 */
#if DEBUG(VERB)
#define VERB(...)                  log_print(LOG_NAME(LOG_MODULE), 0,   DEBUG_VERB, LOG_COLOR(LOG_MODULE), LOG_OUTPUT(LOG_MODULE), ##__VA_ARGS__)
#define VERB_OPT(opt, ...)         log_print(LOG_NAME(LOG_MODULE), opt, DEBUG_VERB, LOG_COLOR(LOG_MODULE), LOG_OUTPUT(LOG_MODULE), ##__VA_ARGS__)
#else
#define VERB(...)                  do { } while(0)
#define VERB_OPT(...)              do { } while(0)
#endif

/**
 * info api, with time added if needed
 */
#if DEBUG(INFO)
#define INFO(fmt, ...)             log_print(LOG_NAME(LOG_MODULE), 0,   DEBUG_INFO, LOG_COLOR(LOG_MODULE), LOG_OUTPUT(LOG_MODULE), fmt, ##__VA_ARGS__)
#define INFO_OPT(opt, fmt, ...)    log_print(LOG_NAME(LOG_MODULE), opt, DEBUG_INFO, LOG_COLOR(LOG_MODULE), LOG_OUTPUT(LOG_MODULE), fmt, ##__VA_ARGS__)
#else
#define INFO(...)                  do { } while(0)
#define INFO_OPT(...)              do { } while(0)
#endif

/**
 * error api
 */
#if DEBUG(ERROR)
#define ERROR(fmt, ...)            log_print(LOG_NAME(LOG_MODULE), 0,   DEBUG_ERROR, COLOR_RED, LOG_OUTPUT(LOG_MODULE), fmt, ##__VA_ARGS__)
#define ERROR_OPT(opt, fmt, ...)   log_print(LOG_NAME(LOG_MODULE), opt, DEBUG_ERROR, COLOR_RED, LOG_OUTPUT(LOG_MODULE), fmt, ##__VA_ARGS__)
#else
#define ERROR(...)                 do { } while(0)
#define ERROR_OPT(...)             do { } while(0)
#endif

