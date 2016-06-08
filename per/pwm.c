/**
 * @file pwm.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_PWM != 0

#include "pwm.h"


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
 * Initialize the PWm module
 */
void pwm_init(void)
{
    psp_pwm_init();
}

/**
 * Set a new PWM period
 * @param pwm the ID of a PWM module (HW_PWMx)
 * @param period_us the new period time in microseconds
 */
void pwm_set_period(pwm_t pwm, uint32_t us)
{
    psp_pwm_set_period(pwm, us);
}

/**
 * Set the duty cycle for a PWM module
 * @param pwm  the ID of a PWM module (HW_PWMx)
 * @param us the new duty cycle in microseconds
 */
void pwm_set_duty_cycle(pwm_t pwm, uint32_t us)
{
    psp_pwm_set_duty_cycle(pwm, us);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*USE_PWM != 0*/