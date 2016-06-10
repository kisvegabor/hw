/**
 * @file psp_tmr.h
 * 
 */

#ifndef PSP_TMR_H
#define PSP_TMR_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"

#if USE_TMR != 0

#include "hw/hw.h"
#include <stdint.h>
#include <stdbool.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef enum
{
    HW_TMR1 = 0,
    HW_TMR2,
    HW_TMR3,
    HW_TMR4,
    HW_TMR5,
    HW_TMR6,
    HW_TMR_NUM,
    HW_TMRX = 0xFF /*TMRX means invalid/unused timer*/
}tmr_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void psp_tmr_init(void);
hw_res_t psp_tmr_set_period(tmr_t tmr, uint32_t p_us);
void psp_tmr_set_cb(tmr_t tmr, void (*cd) (void));
void psp_tmr_en_int(tmr_t tmr, bool en);
void psp_tmr_run(tmr_t tmr, bool en);

/**********************
 *      MACROS
 **********************/

#endif

#endif
