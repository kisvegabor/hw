/**
 * @file timer.h
 * 
 */

#ifndef TIMER_H
#define TIMER_H

/*********************
 *      INCLUDES
 *********************/
#include "../../hw_conf.h"

#if USE_TMR != 0
#include <stdint.h>
#include <stdbool.h>
#include "hw/hw.h"
#include "psp/psp_tmr.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void tmr_init(void);
hw_res_t tmr_set_period(tmr_t tmr, uint32_t period);
void tmr_set_cb(tmr_t tmr, void (*cd) (void));
void tmr_run(tmr_t tmr, bool en);
void tmr_en_int(tmr_t tmr, bool en);

/**********************
 *      MACROS
 **********************/

#endif

#endif
