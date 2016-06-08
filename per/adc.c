/**
 * @file adc.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_ADC != 0

#include "psp/psp_adc.h"

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
 * Initialzie the ADC
 */
void adc_init(void)
{
    psp_adc_init();
}

/**
 * Set an analog channel
 * @param ch analog channel id
 */
void adc_set_ch(uint8_t ch)
{
    psp_adc_set_ch(ch);
}

/**
 * Make conversation on the set channel
 * @return result of the conversation
 */
uint16_t adc_read(void)
{
    return psp_adc_read();
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif  /*USE_ADC != 0*/
