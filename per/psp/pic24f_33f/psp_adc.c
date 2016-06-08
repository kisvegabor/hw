/**
 * @file psp_adc.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/


#include "hw_conf.h"
#if USE_ADC != 0
#include <xc.h>
#include "../psp_adc.h"
#include "misc/os/tick.h"


/*********************
 *      DEFINES
 *********************/
#define PSP_ADC_RES     10 /* bit resolution*/

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
void psp_adc_init(void)
{
    /* Initialize and enable ADC module */
    AD1CON1bits.ASAM = 1;   /*SAMP auto set*/
    AD1CON3bits.ADRC = 1;
    AD1CON2 = 0x0000;
    AD1CON3bits.ADRC = 1;   /*Use the internal RC*/
    AD1CHS0 = 0;            /*Default ch is AN0*/
    AD1CON1bits.ADON = 1;
}

/**
 * Set an analog channel
 * @param ch analog channel id
 */
void psp_adc_set_ch(uint8_t ch)
{
    AD1CHS0 = ch;
}

/**
 * Make conversation on the set channel
 * @return result of the conversation
 */
uint16_t psp_adc_read(void)
{
    AD1CON1bits.SAMP = 0;   /* Start the conversion*/
    while (!AD1CON1bits.DONE); 
    return ADC1BUF0 << (ADC_WORD_LENGTH - PSP_ADC_RES); 
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif  /*USE_ADC != 0*/