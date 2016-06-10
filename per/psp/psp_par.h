/**
 * @file psp_par.h
 * 
 */

#ifndef PSP_PAR_H
#define PSP_PAR_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"

#if USE_PARALLEL != 0 

#include <stdint.h>
#include "hw/hw.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void psp_par_init(void);
void psp_par_set_wait_time(uint8_t wait);  /*PSP_PAR_SLOW to slow mode*/
void psp_par_wr_array(uint32_t adr, const void * buf, uint32_t length);
void psp_par_rd_array(uint32_t adr, void * buf, uint32_t length);

/**********************
 *      MACROS
 **********************/

#endif

#endif
