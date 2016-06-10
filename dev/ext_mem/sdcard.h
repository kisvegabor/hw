/**
 * @file sdcard.h
 * 
 */

#ifndef SDCARD_H
#define SDCARD_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_SDCARD != 0

#include "diskio.h"
#include <stdbool.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void sdcard_init(void);
bool sdcard_ready(void);

/**********************
 *      MACROS
 **********************/

#endif /*USE_SDCARD != 0*/

#endif 