/**
 * @file pcf8574.h
 * 
 */

#ifndef PCF8574_H
#define PCF8574_H

#ifdef __cplusplus
extern "C" {
#endif


/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"

#if USE_PCF8574 != 0
    
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
    
void pcf8574_init(void);
void pcf8574_pin_set(uint8_t pin);
void pcf8574_pin_clr(uint8_t pin);
uint8_t pcf8574_pin_get(uint8_t pin);

/**********************
 *      MACROS
 **********************/
    
#endif /*USE_PCF8574 != 0*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
