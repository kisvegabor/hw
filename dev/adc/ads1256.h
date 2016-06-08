/**
 * @file ads1256.h
 * 
 */

#ifndef ADS1256_H
#define ADS1256_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_ADS1256 != 0

#include <stdint.h>
#include "../device.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void ads1256_init(void);
int32_t ads1256_read(void);

/**********************
 *      MACROS
 **********************/

#endif

#endif
