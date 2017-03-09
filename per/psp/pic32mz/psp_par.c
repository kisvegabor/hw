/**
 * @file psp_par.h
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"

#if USE_PARALLEL != 0 && PSP_PIC32MZ != 0
#include <xc.h>
#include "../psp_par.h"
#include "hw/per/tick.h"

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
    
/**
 * Initialize the parallel port
 */
void psp_par_init(void)
{
    PMCON = 0;
    
    PMMODEbits.MODE = 0b10; /*Mode 2*/
    
    PMCONbits.PTWREN = 1;
    PMCONbits.PTRDEN = 1;
    
    PMMODEbits.MODE16 = 1;
    PMMODEbits.WAITB = PAR_WAITB - 1;
    PMMODEbits.WAITM = PAR_WAITM - 1;
    PMMODEbits.WAITE = PAR_WAITE - 1;
    
    PMCONbits.ON = 1;
}

/**
 * Set the wait time
 * @param wait length of a wr/rd strobe in clock cycles 
 */
void psp_par_set_wait_time(uint8_t wait)
{
    act_wait = wait;  
}

/**
 * Write an array to the parallel port
 * @param adr start address of writing
 * @param buf pointer to the array to write
 * @param length length of the array in words
 */
void psp_par_wr_array(uint32_t adr, const void * buf, uint32_t length)
{
    uint32_t i;
    uint16_t * buf16_p = (uint16_t *) buf;
    for(i = 0; i < length; i++) {
        while(PMMODEbits.BUSY != 0);
        PMDIN = buf16_p[i];
    }
}

/**
 * Read data from the parallel port
 * @param adr start address of reading
 * @param buf point to budder to store the result
 * @param length number of words to read
 */
void psp_par_rd_array(uint32_t adr, void * buf, uint32_t length)
{
    
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
