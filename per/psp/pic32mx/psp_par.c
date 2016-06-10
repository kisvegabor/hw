/**
 * @file psp_par.h
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"

#if USE_PARALLEL != 0 && PSP_PIC32MX != 0
#include <xc.h>
#include "../psp_par.h"

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
static uint8_t act_wait = 0;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
    
void psp_par_init(void)
{
    
}

hw_res_t psp_par_set_wait_time(uint8_t wait)
{
    hw_res_t res = HW_RES_OK;
    
    act_wait = wait;  
    
    return res;
}


void psp_par_wr_array(uint32_t adr, const void * buf, uint32_t length)
{
    
}

void psp_par_rd_array(uint32_t adr, void * buf, uint32_t length)
{
    
}


/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
