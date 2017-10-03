/**
 * @file hx711.h
 * 
 */

#ifndef HX711_H
#define HX711_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_HX711 != 0
    
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
/**
 * Initialize the HX711 AD converter
 */
void hx711_init(void);

/**
 * Read the ADC
 * @return 24 bit value
 */
uint32_t hx711_meas(void);

/**********************
 *      MACROS
 **********************/

#endif /*USE_HX711*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
