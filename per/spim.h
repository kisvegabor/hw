/**
 * @file spi.h
 * 
 */

#ifndef SPIM_H
#define SPIM_H

/*********************
 *      INCLUDES
 *********************/
#include "../../hw_conf.h"
#if USE_SPIM != 0

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
hw_res_t spim_init(void);
hw_res_t spim_cs_en();
hw_res_t spim_cs_dis();
hw_res_t spim_xchg(void * tx_buf, void * rx_buf, uint32_t length);


/**********************
 *      MACROS
 **********************/

#endif

#endif
