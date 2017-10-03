/**
 * @file hcsr04.h
 * 
 */

#ifndef HCSR04_H
#define HCSR04_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_HCSR04 != 0
    
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
 * initialize the HC-SR04 ultrasonic proximity measure module
 */
void hcsr04_init(void);

/**
 * Measure distance
 * @return Distance of an object im cm
 */
uint16_t hcsr04_meas(void);
    
/**********************
 *      MACROS
 **********************/

#endif  /*USE_HCSR04*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /*HCSR04_H*/
