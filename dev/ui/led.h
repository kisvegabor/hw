/**
 * @file led.h
 * 
 */

#ifndef LED_H
#define LED_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_LED != 0

#include <stdint.h>
#include "hw/hw.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum
{
    LED1 = 0,
    LED2,
    LED3,
    LED4,
    LED5,
    LED6,
    LED7,
    LED8,
    LED_NUM
}led_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void led_init(void);
hw_res_t led_on(led_t led);
hw_res_t led_off(led_t led);
hw_res_t led_toggle(led_t led);

/**********************
 *      MACROS
 **********************/

#endif

#endif
