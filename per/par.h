/**
 * @file parallel.h
 * 
 */

#ifndef PARALLEL_H
#define PARALLEL_H

/*********************
 *      INCLUDES
 *********************/
#include "../../hw_conf.h"
#if USE_PARALLEL != 0

#include "hw/hw.h"
#include "psp/psp_par.h"
#include <stdint.h>

/*********************
 *      DEFINES
 *********************/
#define PAR_SLOW 0xFF

/**********************
 *      TYPEDEFS
 **********************/
typedef enum
{
    PAR_CS1 = 0,
    PAR_CS2,
    PAR_NUM
}par_cs_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void par_init(void);
void par_set_wait_time(uint8_t wait);
void par_cs_en(par_cs_t cs);
void par_cs_dis(par_cs_t cs);
void par_wr(uint16_t data);
void par_wr_array(uint16_t * data_p, uint32_t size);
void par_wr_mult(uint16_t  data, uint32_t mult);

/**********************
 *      MACROS
 **********************/

#endif

#endif
