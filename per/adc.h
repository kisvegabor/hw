/**
 * @file adc.h
 * 
 */

#ifndef ADC_H
#define ADC_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_ADC != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void adc_init(void);
void adc_set_ch(uint8_t ch);
uint16_t adc_read(void);

/**********************
 *      MACROS
 **********************/

#endif /*USE_ADC != 0*/

#endif