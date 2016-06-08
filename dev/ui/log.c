/**
 * @file log.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "misc_conf.h"
#if USE_LOG != 0

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "log.h"
#include "hw/per/serial.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void log_wr(const char * txt);

/**********************
 *  STATIC VARIABLES
 **********************/
LOG_FN("log.c");

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Init the logging.
 */
void log_init(void)
{
#if LOG_USE_SERIAL != 0
    serial_set_baud(LOG_SERIAL_DRV, LOG_SERIAL_BAUD);
#endif
    
    SMSG("Message log active");
    SWARN("Warning log active");
    SERR("Error log active");
}

/**
 * Log a message
 * @param path path or file name of file where the log occurred
 * @param func_name name of the function where the log occurred
 * @param format free format string with a text (like in printf)
 * @param ...
 */
void log_msg(const char * path, const char * func_name, const char * format, ...)
{
#if LOG_LEVEL > 2
    char buf[LOG_LENGTH_MAX];

	va_list va;
	va_start(va, format);
	vsprintf(buf,format, va);
	va_end(va);

	log_wr(LOG_SYM_MSG);
	log_wr(path);
	log_wr("/");
	log_wr(func_name);
	log_wr(": ");
	log_wr(buf);
	log_wr("\r\n");
#endif
}


/**
 * Log a warning
 * @param path path or file name of file where the log occurred
 * @param func_name name of the function where the log occurred
 * @param format free format string with a text (like in printf)
 * @param ...
 */
void log_warn(const char * path, const char * func_name, const char * format, ...)
{   
#if LOG_LEVEL > 1
    char buf[LOG_LENGTH_MAX];

	va_list va;
	va_start(va, format);
	vsprintf(buf,format,va);
	va_end(va);

	log_wr(LOG_SYM_WARN);
	log_wr(path);
	log_wr("/");
	log_wr(func_name);
	log_wr(": ");
	log_wr(buf);
	log_wr("\r\n");
#endif
}

/**
 * Log an error
 * @param path path or file name of file where the log occurred
 * @param func_name name of the function where the log occurred
 * @param format free format string with a text (like in printf)
 * @param ...
 */
void log_err(const char * path, const char * func_name, const char * format, ...)
{
#if LOG_LEVEL > 0
    char buf[LOG_LENGTH_MAX];

	va_list va;
	va_start(va, format);
	vsprintf(buf,format,va);
	va_end(va);

	log_wr(LOG_SYM_ERR);
	log_wr(path);
	log_wr("/");
	log_wr(func_name);
	log_wr(": ");
	log_wr(buf);
	log_wr("\r\n");
#endif
}



/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Write out a log
 * @param txt
 */
static void log_wr(const char * txt)
{
#if LOG_USE_SERIAL != 0
    serial_send_force(LOG_SERIAL_DRV, txt, strlen(txt));
#endif

#if LOG_USE_PRINTF != 0
    printf(txt);
#endif
}

#endif
