/**
 * @file tmr.c
 * 
 */



/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_TMR != 0

#include <stddef.h>
#include "hw/hw.h"
#include "tmr.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the timers
 */
void tmr_init(void)
{
    psp_tmr_init();
}

/**
 * Set the period of a timer in microseconds
 * @param tmr the id of a timer (HW_TMRx)
 * @param p_us period in microseconds
 * @return HW_RES_OK or any error from hw_res_t
 */
hw_res_t tmr_set_period(tmr_t tmr, uint32_t p_us)
{
    hw_res_t res;
    
    res = psp_tmr_set_period(tmr, p_us);
    
    return res;
}

/**
 * Set the callback function of a timer interrupt
 * @param tmr the id of a timer (HW_TMRx)
 * @param cb function to call in the timer intrrupts
 */
void tmr_set_cb(tmr_t tmr, void (*cb) (void))
{
    psp_tmr_set_cb(tmr, cb);
    
    if(cb == NULL) psp_tmr_en_int(tmr, false);
    else psp_tmr_en_int(tmr, true);  
}

/**
 * Enable the running of timer
 * @param tmr the id of a timer (HW_TMRx)
 * @param en true: the timer is running
 * @return HW_RES_OK or any error from hw_res_t 
 */
void tmr_run(tmr_t tmr, bool en)
{
    psp_tmr_run(tmr, en);
}

/**
 * Enable the interrupt of a timer
 * @param tmr the id of a timer (HW_TMRx)
 * @param en true: enable the interrupt 
 * @return HW_RES_OK or any error from hw_res_t 
 */
void tmr_en_int(tmr_t tmr, bool en)
{
    psp_tmr_en_int(tmr, en);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
