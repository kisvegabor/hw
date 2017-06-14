/**
 * @file psp_tmr.c
 */

/***********************
 *       INCLUDES
 ***********************/
#include "hw_conf.h"

#if USE_TMR != 0 && PSP_KEA != 0

#include <stddef.h>
#include "derivative.h"         /* include peripheral declarations SKEAZ128M4 */
#include "../../tmr.h"
#include "hw/per/tick.h"


/***********************
 *       DEFINES
 ***********************/
#define IC_PER_US   		((uint32_t)CLOCK_PERIPH / 1000000U) /*Instructions in one usec*/
#define MAX_PERIOD_US		(214748364) 						/*Max possible timer period in usec. (4^32 / IC_PER_US)*/
#define TMR_PRIO_CONV(x) 	(x-2 < 0 ? 0 : x-2)

#ifdef PIT_LDVAL7
#define MCU_TIMER_NUM 8
#elif defined(PIT_LDVAL6)
#define MCU_TIMER_NUM 7
#elif defined(PIT_LDVAL5)
#define MCU_TIMER_NUM 6
#elif defined(PIT_LDVAL4)
#define MCU_TIMER_NUM 5
#elif defined(PIT_LDVAL3)
#define MCU_TIMER_NUM 4
#elif defined(PIT_LDVAL2)
#define MCU_TIMER_NUM 3
#elif defined(PIT_LDVAL1)
#define MCU_TIMER_NUM 2
#endif

/***********************
 *       TYPEDEFS
 ***********************/

typedef struct
{
    uint32_t period;
    void (*cb)(void);
}m_dsc_t;


/***********************
 *   STATIC VARIABLES
 ***********************/
static m_dsc_t dsc[HW_TMR_NUM];

/***********************
 *   GLOBAL PROTOTYPES
 ***********************/
        
/***********************
 *   STATIC PROTOTYPES
 ***********************/
static bool psp_tmr_id_test(tmr_t id);

/***********************
 *   GLOBAL FUNCTIONS
 ***********************/

/**
 * Initialize the timers
 */
void psp_tmr_init(void)
{
#if TMR0_EN && MCU_TIMER_NUM > 0
	NVIC_ClearPendingIRQ(PIT_CH0_IRQn); 		 				/* Clear any Pending IRQ for all PIT ch2 */
	NVIC_EnableIRQ(PIT_CH0_IRQn);       		 				/* Set Enable IRQ for PIT_CH2 */
	NVIC_SetPriority(PIT_CH0_IRQn,TMR_PRIO_CONV(TMR0_PRIO));    /* Set Priority for PIT_CH2 */
#endif

#if TMR1_EN && MCU_TIMER_NUM > 1
	NVIC_ClearPendingIRQ(PIT_CH1_IRQn);  		 				/* Clear any Pending IRQ for all PIT ch2 */
	NVIC_EnableIRQ(PIT_CH1_IRQn);        		 				/* Set Enable IRQ for PIT_CH2 */
	NVIC_SetPriority(PIT_CH1_IRQn,TMR_PRIO_CONV(TMR1_PRIO));    /* Set Priority for PIT_CH2 */
#endif

#if TMR2_EN && MCU_TIMER_NUM > 2
	NVIC_ClearPendingIRQ(PIT_CH2_IRQn); 		 				/* Clear any Pending IRQ for all PIT ch3 */
	NVIC_EnableIRQ(PIT_CH2_IRQn);       		 				/* Set Enable IRQ for PIT_CH3 */
	NVIC_SetPriority(PIT_CH2_IRQn,TMR_PRIO_CONV(TMR2_PRIO));    /* Set Priority for PIT_CH3 */
#endif

#if TMR3_EN && MCU_TIMER_NUM > 3
	NVIC_ClearPendingIRQ(PIT_CH3_IRQn);  		 				/* Clear any Pending IRQ for all PIT ch3*/
	NVIC_EnableIRQ(PIT_CH3_IRQn);        		 				/* Set Enable IRQ for PIT_CH3 */
	NVIC_SetPriority(PIT_CH3_IRQn,TMR_PRIO_CONV(TMR3_PRIO));    /* Set Priority for PIT_CH3 */
#endif

#if TMR4_EN && MCU_TIMER_NUM > 4
	NVIC_ClearPendingIRQ(PIT_CH4_IRQn);  		 				/* Clear any Pending IRQ for all PIT ch4 */
	NVIC_EnableIRQ(PIT_CH4_IRQn);       		 				/* Set Enable IRQ for PIT_CH4 */
	NVIC_SetPriority(PIT_CH4_IRQn,TMR_PRIO_CONV(TMR4_PRIO));    /* Set Priority for PIT_CH4 */
#endif

#if TMR5_EN && MCU_TIMER_NUM > 5
	NVIC_ClearPendingIRQ(PIT_CH5_IRQn);  		 				/* Clear any Pending IRQ for all PIT ch5 */
	NVIC_EnableIRQ(PIT_CH5_IRQn);       		 				/* Set Enable IRQ for PIT_CH5 */
	NVIC_SetPriority(PIT_CH5_IRQn,TMR_PRIO_CONV(TMR5_PRIO));    /* Set Priority for PIT_CH5 */
#endif

#if TMR5_EN && MCU_TIMER_NUM > 6
	NVIC_ClearPendingIRQ(PIT_CH6_IRQn);  		 				/* Clear any Pending IRQ for all PIT ch6 */
	NVIC_EnableIRQ(PIT_CH6_IRQn);       		 				/* Set Enable IRQ for PIT_CH6 */
	NVIC_SetPriority(PIT_CH6_IRQn,TMR_PRIO_CONV(TMR6_PRIO));    /* Set Priority for PIT_CH6 */
#endif

#if TMR5_EN && MCU_TIMER_NUM > 7
	NVIC_ClearPendingIRQ(PIT_CH7_IRQn);  		 				/* Clear any Pending IRQ for all PIT ch7 */
	NVIC_EnableIRQ(PIT_CH7_IRQn);       		 				/* Set Enable IRQ for PIT_CH7 */
	NVIC_SetPriority(PIT_CH7_IRQn,TMR_PRIO_CONV(TMR7_PRIO));    /* Set Priority for PIT_CH7 */
#endif

	SIM_SCGC |= SIM_SCGC_PIT_MASK;     /* Enable bus clock to PIT module */

	PIT_MCR = 0x0;                     /* Turn on PIT module, Freeze disabled */
}

/**
 * Set the period of a timer in microseconds
 * @param tmr the id of a timer (HW_TMRx)
 * @param p_us period in microseconds
 * @return HW_RES_OK or any error from hw_res_t
 */
hw_res_t psp_tmr_set_period(tmr_t tmr, uint32_t p_us)
{
    if(psp_tmr_id_test(tmr) == false) return HW_RES_NOT_EX;
    
    hw_res_t res = HW_RES_OK;

    if(p_us > MAX_PERIOD_US) p_us = MAX_PERIOD_US;

    dsc[tmr].period = p_us;
    PIT->CHANNEL[tmr].LDVAL = p_us * IC_PER_US;

    return res;
}

/**
 * Set the current value of a timer
 * @param tmr the id of a timer (HW_TMRx)
 * @param value the new value
 */
void psp_tmr_set_value(tmr_t tmr, uint32_t value)
{

	/*Not supported by the MCU*/
	return;
}

/**
 * Set the callback function of timer (called in its interrupt)
 * @param tmr the id of a timer (HW_TMRx)
 * @param cb
 */
void psp_tmr_set_cb(tmr_t tmr, void (*cb) (void))
{
    if(psp_tmr_id_test(tmr) == false) return;

    dsc[tmr].cb = cb;
}

/**
 * Enable the interrupt of a timer
 * @param tmr the id of a timer (HW_TMRx)
 * @param en true: interrupt enable, false: disable
 */
void psp_tmr_en_int(tmr_t tmr, bool en)
{
    if(psp_tmr_id_test(tmr) == false) return;

	if(en == false) {
		PIT->CHANNEL[tmr].TCTRL &= (~PIT_TCTRL_TIE_MASK);		/* Disable interrupt */
	} else {
		PIT->CHANNEL[tmr].TCTRL |= PIT_TCTRL_TIE_MASK;  		/* Enable (start) interrupt */
	}
}

/**
 * Enable the running of a timer
 * @param tmr the id of a timer (HW_TMRx)
 * @param en true: timer run, false timer stop
 */
void psp_tmr_run(tmr_t tmr, bool en)
{
    if(psp_tmr_id_test(tmr) == false) return;

	if(en == false) {
		PIT->CHANNEL[tmr].TCTRL &= (~PIT_TCTRL_TEN_MASK);	/* Stop timer */
	} else {
		PIT->CHANNEL[tmr].TCTRL |= PIT_TCTRL_TEN_MASK;  		/* Enable (start) timer */
	}
}

/**
 * 
 */
#if TMR0_EN && MCU_TIMER_NUM > 0
void PIT_CH0_IRQHandler (void) {
	PIT_TFLG0 |= PIT_TFLG_TIF_MASK; 		/* Clear PIT0 flag */
	if(dsc[0].cb != NULL) dsc[0].cb();
}
#endif

#if TMR1_EN && MCU_TIMER_NUM > 1
void PIT_CH1_IRQHandler (void) {
	PIT_TFLG1 |= PIT_TFLG_TIF_MASK; 		/* Clear PIT1 flag */
	if(dsc[1].cb != NULL) dsc[1].cb();
}
#endif

#if TMR2_EN && MCU_TIMER_NUM > 2
void PIT_CH2_IRQHandler (void) {
	PIT_TFLG2 |= PIT_TFLG_TIF_MASK; 		/* Clear PIT2 flag */
	if(dsc[2].cb != NULL) dsc[2].cb();
}
#endif

#if TMR3_EN && MCU_TIMER_NUM > 3
void PIT_CH3_IRQHandler (void) {
	PIT_TFLG3 |= PIT_TFLG_TIF_MASK; 		/* Clear PIT3 flag */
	if(dsc[3].cb != NULL) dsc[3].cb();
}
#endif

#if TMR4_EN && MCU_TIMER_NUM > 4
void PIT_CH4_IRQHandler (void) {
	PIT_TFLG4 |= PIT_TFLG_TIF_MASK; 		/* Clear PIT4 flag */
	if(dsc[4].cb != NULL) dsc[4].cb();
}
#endif

#if TMR5_EN && MCU_TIMER_NUM > 5
void PIT_CH5_IRQHandler (void) {
	PIT_TFLG5 |= PIT_TFLG_TIF_MASK; 		/* Clear PIT5 flag */
	if(dsc[5].cb != NULL) dsc[5].cb();
}
#endif

#if TMR6_EN && MCU_TIMER_NUM > 6
void PIT_CH6_IRQHandler (void) {
	PIT_TFLG6 |= PIT_TFLG_TIF_MASK; 		/* Clear PIT6 flag */
	if(dsc[6].cb != NULL) dsc[6].cb();
}
#endif


#if TMR7_EN && MCU_TIMER_NUM > 7
void PIT_CH7_IRQHandler (void) {
	PIT_TFLG7 |= PIT_TFLG_TIF_MASK; 		/* Clear PIT7 flag */
	if(dsc[7].cb != NULL) dsc[7].cb();
}
#endif


/***********************
 *   STATIC FUNCTIONS
 ***********************/

/**
 * Test a timer id
 * @param id the id of a timer (HW_TMRx)
 * @return true: id is valid, false: id is invalid
 */
static bool psp_tmr_id_test(tmr_t id)
{
	if(sizeof(PIT->CHANNEL[0]) * id >= sizeof(PIT->CHANNEL)) {
		return false;
	}

	return true;
}

#endif
