/**
 * @file pwm.h
 * 
 */

#ifndef PWM_H
#define PWM_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_PWM != 0

#include "psp/psp_pwm.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void pwm_init(void);
void pwm_set_period(pwm_t pwm, uint32_t us);
void pwm_set_duty_cycle(pwm_t pwm, uint32_t us);

/**********************
 *      MACROS
 **********************/

#endif /*USE_PWM != 0*/

#endif