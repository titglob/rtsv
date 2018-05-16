#ifndef LOG_CFG_H
#define LOG_CFG_H

#include <lib_logs.h>

#define LOG_CFG_OPEN_FILE               0
#define LOG_CFG_FILENAME                "log"
#define LOG_CFG_MAX_TAB                 20
#define LOG_CFG_MAX_LOG_LENGTH          256
#define LOG_CFG_MAX_MOD_LENGTH          7
#define LOG_CFG_MAX_MOD_FMT             "%-7s"

#define LOG_CFG_DEBUG_BUS_ENABLED       CPU_CFG_DEBUG_BUS_ENABLED
#define LOG_CFG_DEBUG_BUS_TABLE         0x7009B000                         // see cpu.ld for leon, it is hard coded. 
#define LOG_CFG_DEBUG_BUS_SIZE          4096                               // reserved 4K for logs

#if LIB != BIONIC_LIB
#define LOG_CFG_COLORED_TRACES          1
#else
#define LOG_CFG_COLORED_TRACES          0
#endif

/**
 * debugging of api
 */
#define _API__LOG_NAME        "API"
#define _API__LOG_LEVEL        DEBUG_ERROR
#define _API__LOG_COLOR        COLOR_MAGENTA
#define _API__LOG_OUTPUT       LOG_DEF_PRINT

/**
 * debugging of sdk
 */
#define _SDK__LOG_NAME         "SDK"
#define _SDK__LOG_LEVEL        DEBUG_ERROR
#define _SDK__LOG_COLOR        COLOR_YELLOW
#define _SDK__LOG_OUTPUT       LOG_DEF_PRINT

#define _SDK_API__LOG_NAME     "SDK_API"
#define _SDK_API__LOG_LEVEL    DEBUG_VERB
#define _SDK_API__LOG_COLOR    COLOR_MAGENTA
#define _SDK_API__LOG_OUTPUT   LOG_DEF_USER

/**
 * debugging of mac
 */
#define _MAC__LOG_NAME         "MAC"
#define _MAC__LOG_LEVEL        DEBUG_ERROR
#define _MAC__LOG_COLOR        COLOR_YELLOW
#define _MAC__LOG_OUTPUT       LOG_DEF_PRINT

/**
 * debugging of operating system
 */
#define _OS__LOG_NAME          "OS"
#define _OS__LOG_LEVEL         DEBUG_ERROR
#define _OS__LOG_COLOR         COLOR_DEFAULT
#define _OS__LOG_OUTPUT        LOG_DEF_PRINT

#define _OS_SIG__LOG_NAME      "OS_SIG"
#define _OS_SIG__LOG_LEVEL     DEBUG_ERROR
#define _OS_SIG__LOG_COLOR     COLOR_DEFAULT
#define _OS_SIG__LOG_OUTPUT    LOG_DEF_PRINT

#define _OS_STK__LOG_NAME      "OS_STK"
#define _OS_STK__LOG_LEVEL     DEBUG_ERROR
#define _OS_STK__LOG_COLOR     COLOR_DEFAULT
#define _OS_STK__LOG_OUTPUT    LOG_DEF_PRINT


/**
 * debugging of processor
 */
#define _CPU__LOG_NAME         "CPU"
#define _CPU__LOG_LEVEL        DEBUG_ERROR
#define _CPU__LOG_COLOR        COLOR_DEFAULT
#define _CPU__LOG_OUTPUT       LOG_DEF_PRINT



/**
 * debugging of board support package
 */
#define _BSP__LOG_NAME         "BSP"
#define _BSP__LOG_LEVEL        DEBUG_ERROR
#define _BSP__LOG_COLOR        COLOR_DEFAULT
#define _BSP__LOG_OUTPUT       LOG_DEF_PRINT



/**
 * debugging of heap memory allocations
 */
#define _HEAP__LOG_NAME        "HEAP"
#define _HEAP__LOG_LEVEL       DEBUG_ERROR
#define _HEAP__LOG_COLOR       COLOR_DEFAULT
#define _HEAP__LOG_OUTPUT      LOG_DEF_PRINT



/**
 * debugging of heap memory allocations
 */
#define _POOL__LOG_NAME        "POOL"
#define _POOL__LOG_LEVEL       DEBUG_ERROR
#define _POOL__LOG_COLOR       COLOR_DEFAULT
#define _POOL__LOG_OUTPUT      LOG_DEF_PRINT



/**
 * debugging of network
 */
#define _NET__LOG_NAME         "NET"
#define _NET__LOG_LEVEL        DEBUG_ERROR
#define _NET__LOG_COLOR        COLOR_DEFAULT
#define _NET__LOG_OUTPUT       LOG_DEF_PRINT



/**
 * debugging of filesystem
 */
#define _FS__LOG_NAME          "FS"
#define _FS__LOG_LEVEL         DEBUG_ERROR
#define _FS__LOG_COLOR         COLOR_GRAY
#define _FS__LOG_OUTPUT        LOG_DEF_PRINT

#define _FSC__LOG_NAME         "FSC"
#define _FSC__LOG_LEVEL        DEBUG_ERROR
#define _FSC__LOG_COLOR        COLOR_GRAY
#define _FSC__LOG_OUTPUT       LOG_DEF_PRINT


/**
 * debugging of DEMOD
 */
#define _DEMOD__LOG_NAME         "DEMOD"
#define _DEMOD__LOG_LEVEL        DEBUG_ERROR
#define _DEMOD__LOG_COLOR        COLOR_BLUE
#define _DEMOD__LOG_OUTPUT       LOG_DEF_PRINT
/**
 * debugging of SVMS
 */
#define _SVMS__LOG_NAME          "SVMS"
#define _SVMS__LOG_LEVEL         DEBUG_ERROR
#define _SVMS__LOG_COLOR         COLOR_BLUE
#define _SVMS__LOG_OUTPUT        LOG_DEF_PRINT
/**
 * debugging of RMS
 */
#define _RMS__LOG_NAME           "RMS"
#define _RMS__LOG_LEVEL          DEBUG_ERROR
#define _RMS__LOG_COLOR          COLOR_BLUE
#define _RMS__LOG_OUTPUT         LOG_DEF_PRINT
/**
 * debugging of hal layer
 */
#define _HAL__LOG_NAME           "HAL"
#define _HAL__LOG_LEVEL          DEBUG_ERROR
#define _HAL__LOG_COLOR          COLOR_BROWN
#define _HAL__LOG_OUTPUT         LOG_DEF_PRINT

#define _HAL_CORE__LOG_NAME      "HAL_CORE"
#define _HAL_CORE__LOG_LEVEL     DEBUG_ERROR
#define _HAL_CORE__LOG_COLOR     COLOR_BROWN
#define _HAL_CORE__LOG_OUTPUT    LOG_DEF_PRINT

#define _HAL_CPU__LOG_NAME       "HAL_CPU"
#define _HAL_CPU__LOG_LEVEL      DEBUG_ERROR
#define _HAL_CPU__LOG_COLOR      COLOR_BROWN
#define _HAL_CPU__LOG_OUTPUT     LOG_DEF_PRINT

#define _HAL_COM__LOG_NAME       "HAL_COM"
#define _HAL_COM__LOG_LEVEL      DEBUG_ERROR
#define _HAL_COM__LOG_COLOR      COLOR_BROWN
#define _HAL_COM__LOG_OUTPUT     LOG_DEF_PRINT

#define _HAL_BUS__LOG_NAME       "HAL_BUS"
#define _HAL_BUS__LOG_LEVEL      DEBUG_ERROR
#define _HAL_BUS__LOG_COLOR      COLOR_BROWN
#define _HAL_BUS__LOG_OUTPUT     LOG_DEF_PRINT

#define _HAL_OCTO__LOG_NAME      "HAL_OCTO"
#define _HAL_OCTO__LOG_LEVEL     DEBUG_ERROR
#define _HAL_OCTO__LOG_COLOR     COLOR_BROWN
#define _HAL_OCTO__LOG_OUTPUT    LOG_DEF_PRINT

#define _HAL_REFLEX__LOG_NAME    "HAL_REFLEX"
#define _HAL_REFLEX__LOG_LEVEL   DEBUG_ERROR
#define _HAL_REFLEX__LOG_COLOR   COLOR_BROWN
#define _HAL_REFLEX__LOG_OUTPUT  LOG_DEF_PRINT

#define _HAL_TUNER__LOG_NAME     "HAL_TUNER"
#define _HAL_TUNER__LOG_LEVEL    DEBUG_ERROR
#define _HAL_TUNER__LOG_COLOR    COLOR_BROWN
#define _HAL_TUNER__LOG_OUTPUT   LOG_DEF_PRINT



/**
 * debugging of RF
 */
#define _RF__LOG_NAME          "RF"
#define _RF__LOG_LEVEL         DEBUG_ERROR
#define _RF__LOG_COLOR         COLOR_GREEN
#define _RF__LOG_OUTPUT        LOG_DEF_PRINT


/**
 * debugging of test msg
 */
#define _MSG_TEST__LOG_NAME    "MSG_TEST"
#define _MSG_TEST__LOG_LEVEL   DEBUG_ERROR
#define _MSG_TEST__LOG_COLOR   COLOR_MAGENTA
#define _MSG_TEST__LOG_OUTPUT  LOG_DEF_PRINT


/**
 * default debugging
 */
#ifndef _DEF__LOG_NAME
#define _DEF__LOG_NAME         NULL
#endif
#ifndef _DEF__LOG_LEVEL
#define _DEF__LOG_LEVEL        DEBUG_ERROR
#endif
#ifndef _DEF__LOG_COLOR
#define _DEF__LOG_COLOR        NULL
#endif
#ifndef _DEF__LOG_OUTPUT
#define _DEF__LOG_OUTPUT       LOG_DEF_PRINT
#endif

#include <lib_set_logs.h>

#endif
