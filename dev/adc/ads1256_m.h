/**
 * @file ads1256.h
 * 
 */

#ifndef ADS1256_M_H
#define ADS1256_M_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_ADS1256_M != 0

#include "hw/hw.h"
#include <stdint.h>


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void ads1256_m_init(void);
void ads1256_m_read(int32_t * meas_a);

/**********************
 *      MACROS
 **********************/

#endif

#endif
