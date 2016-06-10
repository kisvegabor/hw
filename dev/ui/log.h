/**
 * @file log.h
 * 
 */

#ifndef LOG_H
#define LOG_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_LOG != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void log_init(void);
void log_msg(const char * path, const char * func_name, const char * format, ...);
void log_warn(const char * path, const char * func_name, const char * format, ...);
void log_err(const char * path, const char * func_name, const char * format, ...);

/**********************
 *      MACROS
 **********************/

#define LOG_FN(fn)  static const char db_fn[] = {fn};      


#if LOG_LEVEL > 0
#define SERR(...) 	log_err(db_fn, __FUNCTION__, __VA_ARGS__)
#else
#define SERR(...)  {}
#endif

#if LOG_LEVEL > 1
#define SWARN(...) 	log_warn(db_fn, __FUNCTION__, __VA_ARGS__)
#else
#define SWARN(...) {}
#endif

#if LOG_LEVEL > 2
#define SMSG(...) 	log_msg(db_fn, __FUNCTION__, __VA_ARGS__)
#else
#define SMSG(...)  {}
#endif

#endif
#endif