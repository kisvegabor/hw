/**
 * @file psp_pwm.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_PWM != 0

#include <stddef.h>
#include <xc.h>
#include "../psp_pwm.h"

/*********************
 *      DEFINES
 *********************/
#define PSP_PWM_NUM (sizeof(mdsc) / sizeof(mdsc[0]))
#define PWM_CNT_IN_US (CLOCK_PERIPH / 1000000)

/**********************
 *      TYPEDEFS
 **********************/
typedef struct
{
    volatile OC1CON1BITS * OCxCON1;
    volatile OC1CON2BITS * OCxCON2;
    volatile unsigned int *  OCxR;
    volatile unsigned int *  OCxRS;
}mdsc_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static mdsc_t mdsc[] = {
/*          OCxCON1                             OCxCON2                 OCxR    OCxRS*/
#if defined(OC1R) && PWM1_EN != 0
{(volatile OC1CON1BITS *) &OC1CON1, (volatile OC1CON2BITS *) &OC1CON2, &OC1R, &OC1RS, },
#else
{ NULL,                             NULL,                               NULL,  NULL},
#endif
#if defined(OC2R) && PWM2_EN != 0
{(volatile OC1CON1BITS *) &OC2CON1, (volatile OC1CON2BITS *) &OC2CON2, &OC2R, &OC2RS, },
#else
{ NULL,                             NULL,                               NULL,  NULL},
#endif
#if defined(OC3R) && PWM3_EN != 0
{(volatile OC1CON1BITS *) &OC3CON1, (volatile OC1CON2BITS *) &OC3CON2, &OC3R, &OC3RS, },
#else
{ NULL,                             NULL,                               NULL,  NULL},
#endif
#if defined(OC4R) && PWM4_EN != 0
{(volatile OC1CON1BITS *) &OC4CON1, (volatile OC1CON2BITS *) &OC4CON2, &OC4R, &OC4RS, },
#else
{ NULL,                             NULL,                               NULL,  NULL},
#endif
#if defined(OC5R) && PWM5_EN != 0
{(volatile OC1CON1BITS *) &OC5CON1, (volatile OC1CON2BITS *) &OC5CON2, &OC5R, &OC5RS, },
#else
{ NULL,                             NULL,                               NULL,  NULL},
#endif
#if defined(OC6R) && PWM6_EN != 0
{(volatile OC1CON1BITS *) &OC6CON1, (volatile OC1CON2BITS *) &OC6CON2, &OC6R, &OC6RS, },
#else
{ NULL,                             NULL,                               NULL,  NULL},
#endif
#if defined(OC7R) && PWM7_EN != 0
{(volatile OC1CON1BITS *) &OC7CON1, (volatile OC1CON2BITS *) &OC7CON2, &OC7R, &OC7RS, },
#else
{ NULL,                             NULL,                               NULL,  NULL},
#endif
#if defined(OC8R) && PWM8_EN != 0
{(volatile OC1CON1BITS *) &OC8CON1, (volatile OC1CON2BITS *) &OC8CON2, &OC8R, &OC8RS, },
#else
{ NULL,                             NULL,                               NULL,  NULL},
#endif
};


/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the PWM periphery
 */
void psp_pwm_init(void)
{
    pwm_t i;
    for(i = HW_PWM1; i < HW_PWM_NUM && i < PSP_PWM_NUM; i++) {
        mdsc[i].OCxCON1->OCM = 0b000;    /* Disable Output Compare Module */
        *mdsc[i].OCxR = PWM_DEF_DUTY_CYCLE * PWM_CNT_IN_US;             
        *mdsc[i].OCxRS = PWM_DEF_PERIOD * PWM_CNT_IN_US;
        mdsc[i].OCxCON1->OCTSEL =  0x07; /* Peripheral clock as the clock input */
        mdsc[i].OCxCON1->OCM = 0b110;    /* Edge Aligned PWM mode */
        mdsc[i].OCxCON2->SYNCSEL=31;     /* Synchronize itself. Illegal but suggested in the datasheet*/
    }
}

/**
 * Set the period in microseconds
 * @param pwm id of the pwm module (from pwm_t)
 * @param period_us period in microseconds
 */
void psp_pwm_set_period(pwm_t pwm, uint32_t period_us)
{
    if(pwm >= HW_PWM_NUM || pwm >= PSP_PWM_NUM) return;
    
    if(mdsc[pwm].OCxRS == NULL) return;
    
    *mdsc[pwm].OCxRS = period_us * PWM_CNT_IN_US;
}

/**
 * Set the duty cycle in microseconds
 * @param pwm id of the pwm module (from pwm_t)
 * @param period_us duty cycle in microseconds
 */
void psp_pwm_set_duty_cycle(pwm_t pwm, uint32_t us)
{
    if(pwm >= HW_PWM_NUM || pwm >= PSP_PWM_NUM ) return;
    
    if(mdsc[pwm].OCxR == NULL) return;
    
    *mdsc[pwm].OCxR = us * PWM_CNT_IN_US;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*USE_PWM != 0*/