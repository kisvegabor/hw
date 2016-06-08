/**
 * @file psp_pwm.h
 * 
 */

#ifndef PSP_PWM_H
#define PSP_PWM_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_PWM != 0

#include "hw/hw.h"
#include <stdint.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum
{
    HW_PWM1,
    HW_PWM2,
    HW_PWM3,
    HW_PWM4,
    HW_PWM5,
    HW_PWM6,
    HW_PWM7,
    HW_PWM8,
    HW_PWM_NUM,
    HW_PWMX = 0xFF,
}pwm_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void psp_pwm_init(void);
void psp_pwm_set_period(pwm_t pwm, uint32_t period_us);
void psp_pwm_set_duty_cycle(pwm_t pwm, uint32_t us);

/**********************
 *      MACROS
 **********************/

#endif   /*USE_PWM != 0*/

#endif