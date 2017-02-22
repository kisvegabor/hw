/**
 * @file timer.c
 */

#include "hw_conf.h"

#if USE_TMR != 0 && PSP_PIC24F_33F != 0

#include <stddef.h>
#include <xc.h>
#include "hw/hw.h"
#include "hw/per/tmr.h"

/***********************
 *       INCLUDES
 ***********************/
#define IC_PER_US   ((uint32_t)CLOCK_PERIPH / 1000000U) //Instructions in a usec

#define TIMER_DEF_PRIO   INT_PRIO_MID 

#define TIMER1_IF IFS0bits.T1IF
#define TIMER1_IE IEC0bits.T1IE
#define TIMER1_IP IPC0bits.T1IP

#define TIMER2_IF IFS0bits.T2IF
#define TIMER2_IE IEC0bits.T2IE
#define TIMER2_IP IPC1bits.T2IP

#define TIMER3_IF IFS0bits.T3IF
#define TIMER3_IE IEC0bits.T3IE
#define TIMER3_IP IPC2bits.T3IP

#define TIMER4_IF IFS1bits.T4IF
#define TIMER4_IE IEC1bits.T4IE
#define TIMER4_IP IPC6bits.T4IP

#define TIMER5_IF IFS1bits.T5IF
#define TIMER5_IE IEC1bits.T5IE
#define TIMER5_IP IPC7bits.T5IP

/***********************
 *       TYPEDEFS
 ***********************/

typedef struct
{
    volatile T1CONBITS * TxCON;
    volatile unsigned int * TMRx;
    volatile unsigned int * PRx;
    uint32_t period;
    void (*cb)(void);
}m_dsc_t;


/***********************
 *   GLOBAL VARIABLES
 ***********************/

/***********************
 *   STATIC VARIABLES
 ***********************/

static m_dsc_t m_dsc[] = 
{           /*TxCON*/                /*TMRx*/       /*PRx*/   /*period*/   /*Callback*/
#if  TMR1_EN != 0
        {(T1CONBITS*)&T1CONbits,     &TMR2,         &PR2,       0,          NULL},
#else
        {NULL,                       NULL,          NULL,       0,          NULL},
#endif
#if TMR2_EN != 0
        {(T1CONBITS*)&T2CONbits,     &TMR2,         &PR2,       0,          NULL},
#else
        {NULL,                       NULL,          NULL,       0,          NULL},
#endif
#if TMR3_EN != 0
        {(T1CONBITS*)&T3CONbits,     &TMR3,         &PR3,       0,          NULL},
#else
        {NULL,                       NULL,          NULL,       0,          NULL},
#endif
#if TMR4_EN != 0
        {(T1CONBITS*)&T4CONbits,     &TMR4,         &PR4,       0,          NULL},
#else
        {NULL,                       NULL,          NULL,       0,          NULL},
#endif
#if TMR5_EN != 0
        {(T1CONBITS*)&T5CONbits,     &TMR5,         &PR5,       0,          NULL},
#else
        {NULL,                       NULL,          NULL,       0,          NULL},
#endif
#if TMR6_EN != 0
        {(T1CONBITS*)&T6CONbits,     &TMR6,         &PR6,       0,          NULL},
#else
        {NULL,                       NULL,          NULL,       0,          NULL},
#endif
};

static const uint16_t tmr_ps[] = {1, 8, 64, 256}; 

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
 * Initialize the timer
 */
void psp_tmr_init(void)
{
    tmr_t t;
    
    for(t = HW_TMR1; t < HW_TMR_NUM; t++) {
        if(psp_tmr_id_test(t) == false) continue;
            *((unsigned int *)m_dsc[t].TxCON) = 0;
            *((unsigned int *)m_dsc[t].TMRx) = 0;
            *((unsigned int *)m_dsc[t].PRx) = 0xFFFF;
    }   
}

/**
 * Set the period of a timer
 * @param tmr id of the timer (from tmr_t enum)
 * @param p_us period in microseconds
 * @return HW_RES_OK or any error from hw_res_t
 */
hw_res_t psp_tmr_set_period(tmr_t tmr, uint32_t p_us)
{
    if(psp_tmr_id_test(tmr) == false) return HW_RES_NOT_EX;
    
    hw_res_t res = HW_RES_OK;
    
    if(res == HW_RES_OK) {
        uint32_t pr = p_us * IC_PER_US;
        uint32_t new_pr = UINT32_MAX;
        uint8_t i;

        for(i = 0; 
            i < sizeof(tmr_ps) / sizeof(tmr_ps[0]) && new_pr > UINT16_MAX; 
            i++)
        {
            new_pr = pr / tmr_ps[i];
        }

        if(new_pr > UINT16_MAX) res = HW_RES_INV_PARAM;
        else{
            /*Set the prescale*/
            *m_dsc[tmr].PRx = new_pr;
            m_dsc[tmr].TxCON->TCKPS = i-1;

            /*Set the clock source*/
            m_dsc[tmr].TxCON->TCS = 0;
        }
    }
    return res;
}
    
/**
 * Set the current value of the timer
 * @param tmr id of the timer (from tmr_t enum)
 * @param value the new value
 */
void psp_tmr_set_value(tmr_t tmr, uint32_t value)
{
    if(psp_tmr_id_test(tmr) == false) return;
    
    *m_dsc[tmr].TMRx = value;
}

/**
 * Set a function to call it in the timer interrupt
 * @param tmr id of the timer (from tmr_t enum)
 * @param cb pointer to a void (*) (void) function
 */
void psp_tmr_set_cb(tmr_t tmr, void (*cb) (void))
{
    if(psp_tmr_id_test(tmr) == false) return;
    
    m_dsc[tmr].cb = cb;
}

/**
 * Enable/disable the running of a timer
 * @param tmr id of the timer (from tmr_t enum)
 * @param en true: timer run, false: timer stop
 */
void psp_tmr_run(tmr_t tmr, bool en)
{
    if(psp_tmr_id_test(tmr) == false) return;
   
    m_dsc[tmr].TxCON->TON = (en == false ? 0 : 1);
}

/**
 * Enable/disable the timer interrupt
 * @param tmr id of the timer (from tmr_t enum)
 * @param en true: timer interrupt enable, false: timer interrupt disable
 */
void psp_tmr_en_int(tmr_t tmr, bool en)
{
    if(psp_tmr_id_test(tmr) == false) return;
    
    uint8_t en_value = (en == false ?  0 : 1);
            
    switch(tmr)
    {
#if TMR1_EN != 0
        case HW_TMR1:
            TIMER1_IE = en_value;
            TIMER1_IP = TMR1_PRIO;
            break;
#endif

#if TMR2_EN != 0
        case HW_TMR2:
            TIMER2_IE = en_value;
            TIMER2_IP = TMR2_PRIO;
            break;
#endif

#if TMR3_EN != 0
        case HW_TMR3:
            TIMER3_IE = en_value;
            TIMER3_IP = TMR3_PRIO;
            break;
#endif

#if TMR4_EN != 0
        case HW_TMR4:
            TIMER4_IE = en_value;
            TIMER4_IP = TMR4_PRIO;
            break;
#endif

#if TMR5_EN != 0
        case HW_TMR5:
            TIMER5_IE = en_value;
            TIMER5_IP = TMR5_PRIO;
            break;
#endif

#if TMR6_EN != 0
        case HW_TMR6:
            TIMER6_IE = en_value;
            TIMER6_IP = TIMER6_PRIO;
            break;
#endif
        default:
            break;
    }
}

/**
 * Timer 1 interrupt handler
 */
#if TMR1_EN != 0
void __attribute__((__interrupt__, auto_psv )) _ISR _T1Interrupt (void)
{
    TIMER1_IF = 0;
    
    if(m_dsc[HW_TMR1].cb != NULL) {
        m_dsc[HW_TMR1].cb();
    }
}
#endif

/**
 * Timer 2 interrupt handler
 */
#if TMR2_EN != 0
void __attribute__((__interrupt__, auto_psv )) _ISR _T2Interrupt (void)
{
    TIMER2_IF = 0;
    
    if(m_dsc[HW_TMR2].cb != NULL) {
        m_dsc[HW_TMR2].cb();
    }
}
#endif

/**
 * Timer 3 interrupt handler
 */
#if TMR3_EN != 0
void __attribute__((__interrupt__, auto_psv )) _ISR _T3Interrupt (void)
{
    TIMER3_IF = 0;
    
    if(m_dsc[HW_TMR3].cb != NULL) {
        m_dsc[HW_TMR3].cb();
    }
    
}
#endif

/**
 * Timer 4 interrupt handler
 */
#if TMR4_EN != 0
void __attribute__((__interrupt__, auto_psv )) _ISR _T4Interrupt (void)
{
    TIMER4_IF = 0;
    
    if(m_dsc[HW_TMR4].cb != NULL)
    {
        m_dsc[HW_TMR4].cb();
    }
}
#endif

/**
 * Timer 5 interrupt handler
 */
#if TMR5_EN != 0
void __attribute__((__interrupt__, auto_psv )) _ISR _T5Interrupt (void)
{
    TIMER5_IF = 0;
    
    if(m_dsc[HW_TMR5].cb != NULL)
    {
        m_dsc[HW_TMR5].cb();
    }
}
#endif

/**
 * Timer 6 interrupt handler
 */
#if TMR6_EN != 0
void __attribute__((__interrupt__, auto_psv )) _ISR _T6Interrupt (void)
{
    TIMER6_IF = 0;
    
    if(m_dsc[HW_TMR6].cb != NULL)
    {
        m_dsc[HW_TMR6].cb();
    }
}
#endif

/***********************
 *   STATIC FUNCTIONS
 ***********************/

/**
 * Test the timer id
 * @param id id of a timer (from tmr_t enum)
 * @return true: id si valid,false: id is invalid
 */
static bool psp_tmr_id_test(tmr_t id)
{
    if(id >= HW_TMR_NUM) return false; 
    else if (m_dsc[id].TxCON == NULL) return false;
    
    return true;
}


#endif
