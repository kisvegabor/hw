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

#include "hw/hw.h"
#include <stdbool.h>
#include "dstore.h"

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

#endif

#endif
