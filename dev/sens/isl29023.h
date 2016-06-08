/**
 * @file isl29023.h
 * 
 */

#ifndef ISL29023_H
#define ISL29023_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_ISL29023 != 0

#include "hw/hw.h"
#include <stdint.h>
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
void isl29023_init(void);
hw_res_t isl29023_start_al(void);
bool isl29023_busy(void);
hw_res_t isl29023_read_al (uint32_t * meas);

/**********************
 *      MACROS
 **********************/

#endif /* USE_ISL29023 != 0 */

#endif