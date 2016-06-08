/**
 * @file psp_adc.h
 * 
 */

#ifndef PSP_ADC_H
#define PSP_ADC_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"

#if USE_ADC != 0
#include <stdint.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void psp_adc_init(void);
void psp_adc_set_ch(uint8_t ch);
uint16_t psp_adc_read(void);

/**********************
 *      MACROS
 **********************/

#endif  /*USE_ADC != 0*/

#endif 