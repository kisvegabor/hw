/**
 * @file psp_tmr.c
 */

/***********************
 *       INCLUDES
 ***********************/
#include "hw_conf.h"

#if USE_TMR != 0 && PSP_PIC32MX != 0

#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING
#include <plib.h>
#include <stddef.h>
#include <xc.h>
#include "../../tmr.h"


/***********************
 *       DEFINES
 ***********************/
#define IC_PER_US   ((uint32_t)CLOCK_PERIPH / 1000000U) //Instructions in a usec

#define TIMER_DEF_PRIO   INT_PRIO_MID 

/* Ignore Timer1
#define TIMER1_IF IFS0bits.T1IF
#define TIMER1_IE IEC0bits.T1IE
#define TIMER1_IP IPC1bits.T1IP
*/
#define TIMER2_IF IFS0bits.T2IF
#define TIMER2_IE IEC0bits.T2IE
#define TIMER2_IP IPC2bits.T2IP

#define TIMER3_IF IFS0bits.T3IF
#define TIMER3_IE IEC0bits.T3IE
#define TIMER3_IP IPC3bits.T3IP

#define TIMER4_IF IFS0bits.T4IF
#define TIMER4_IE IEC0bits.T4IE
#define TIMER4_IP IPC4bits.T4IP

#define TIMER5_IF IFS0bits.T5IF
#define TIMER5_IE IEC0bits.T5IE
#define TIMER5_IP IPC5bits.T5IP

#define IPL_NAME(prio) IPL_CONC(prio)
#define IPL_CONC(prio) IPL ## prio ## AUTO
/***********************
 *       TYPEDEFS
 ***********************/

typedef struct
{
    volatile __T2CONbits_t * TxCON;
    volatile unsigned int * TMRx;
    volatile unsigned int * PRx;
}timer_regs_t;

typedef struct
{
    uint32_t period;
    void (*cb)(void);
}timer_dsc_t;


/***********************
 *   STATIC VARIABLES
 ***********************/

static timer_dsc_t m_dsc[] = 
{
                /*period*/      /*fp*/   
/*TMR1*/   {0,              NULL,},
/*TMR2*/   {0,              NULL,},
/*TMR3*/   {0,              NULL,},
/*TMR4*/   {0,              NULL,},
/*TMR5*/   {0,              NULL,},
/*TMR6*/   {0,              NULL,},
};

static timer_regs_t reg_map[] = 
{           /*TxCON*/                       /*TMRx*/       /*PRx*/
#if 0   /*Always ignore timer 1 bcause its differnet from the otheres*/ 
        {(volatile T1CONBITS*)&T1CONbits,     &TMR1,         &PR2},
#else
        {NULL,                                NULL,          NULL},
#endif
#if TMR2_EN != 0
        {(volatile __T2CONbits_t*)&T2CONbits, &TMR2,         &PR2},
#else
        {NULL,                                NULL,          NULL},
#endif
#if TMR3_EN != 0
        {(volatile __T2CONbits_t*)&T3CONbits, &TMR3,         &PR3},
#else
        {NULL,                                NULL,          NULL},
#endif
#if TMR4_EN != 0
        {(volatile __T2CONbits_t*)&T4CONbits, &TMR4,         &PR4},
#else
        {NULL,                                 NULL,          NULL},
#endif
#if TMR5_EN != 0
        {(volatile __T2CONbits_t*)&T5CONbits,  &TMR5,         &PR5},
#else
        {NULL,                                 NULL,          NULL},
#endif 
#if TMR6_EN != 0
        {(volatile __T2CONbits_t*)&T6CONbits,  &TMR6,         &PR6},
#else
        {NULL,                                 NULL,          NULL},
#endif
};
static const uint16_t tmr_ps[] = {1, 2, 4, 8, 16, 32, 64, 256}; 

/***********************
 *   GLOBAL PROTOTYPES
 ***********************/
        
/***********************
 *   STATIC PROTOTYPES
 ***********************/
static hw_res_t psp_tmr_id_test(tmr_t id);

/***********************
 *   GLOBAL FUNCTIONS
 ***********************/

void psp_tmr_init(void)
{
    
}

hw_res_t psp_tmr_set_period(tmr_t tmr, uint32_t p_us)
{
    hw_res_t res = psp_tmr_id_test(tmr);
    
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

        if(new_pr > UINT16_MAX) {
            res = HW_RES_INV_PARAM;
        } 
        else {
            /*Set the prescale*/
            *reg_map[tmr].PRx = new_pr;
            if(i > 0) {
                i--;
            }

            reg_map[tmr].TxCON->TCKPS = i;    
        }
    }
    
    return res;
}

void psp_tmr_set_value(tmr_t tmr, uint32_t value)
{
    hw_res_t res = psp_tmr_id_test(tmr);
    
    if(res == HW_RES_OK) {
        *reg_map[tmr].TMRx = value;
    }
    
}

void psp_tmr_set_cb(tmr_t tmr, void (*cb) (void))
{
    hw_res_t res = psp_tmr_id_test(tmr);
    
    if(res == HW_RES_OK) {
        m_dsc[tmr].cb = cb;
    }
}

void psp_tmr_en_int(tmr_t tmr, bool en)
{
    uint8_t en_value = (en == false ?  0 : 1);
            
    switch(tmr)
    {
#if 0
        case HW_TMR1:
            TIMER1_IE = en_value;
            TIMER1_IP = TMR1_PRIO;
            break;
#endif

#if TMR2_EN != 0 && TMR2_PRIO != HW_INT_PRIO_OFF
        case HW_TMR2:
            TIMER2_IE = en_value;
            TIMER2_IP = TMR2_PRIO;
            break;
#endif

#if TMR3_EN != 0 && TMR3_PRIO != HW_INT_PRIO_OFF
        case HW_TMR3:
            TIMER3_IE = en_value;
            TIMER3_IP = TMR3_PRIO;
            break;
#endif

#if TMR4_EN != 0 && TMR4_PRIO != HW_INT_PRIO_OFF
        case HW_TMR4:
            TIMER4_IE = en_value;
            TIMER4_IP = TMR4_PRIO;
            break;
#endif

#if TMR5_EN != 0 && TMR5_PRIO != HW_INT_PRIO_OFF
        case HW_TMR5:
            TIMER5_IE = en_value;
            TIMER5_IP = TMR5_PRIO;
            break;
#endif

#if TMR6_EN != 0 && TMR6_PRIO != HW_INT_PRIO_OFF
        case HW_TMR6:
            TIMER6_IE = en_value;
            TIMER6_IP = TMR6_PRIO;
            break;
#endif
        default:
            break;

    }
}

void psp_tmr_run(tmr_t tmr, bool en)
{
    hw_res_t res = psp_tmr_id_test(tmr);
    if(res == HW_RES_OK) {
        reg_map[tmr].TxCON->ON = (en == false ? 0 : 1);
    }
}

/**
 * 
 */
#if 0
void __ISR(_TIMER_1_VECTOR, IPL_NAME(TMR1_PRIO)) isr_timer1 (void)
{
    TIMER1_IF = 0;
    
    if(m_dsc[HW_TMR1].cb != NULL)
    {
        m_dsc[HW_TMR1].cb();
    }
}
#endif

/**
 * 
 */
#if TMR2_EN != 0
void __ISR(_TIMER_2_VECTOR, IPL_NAME(TMR2_PRIO)) isr_timer2 (void)
{
    TIMER2_IF = 0;

    if(m_dsc[HW_TMR2].cb != NULL)
    {
        m_dsc[HW_TMR2].cb();
    }
}
#endif

/**
 * 
 */
#if TMR3_EN != 0
void __ISR(_TIMER_3_VECTOR, IPL_NAME(TMR3_PRIO)) isr_timer3 (void)
{
    TIMER3_IF = 0;
    
    if(m_dsc[HW_TMR3].cb != NULL)
    {
        m_dsc[HW_TMR3].cb();
    }
    
}
#endif

/**
 * 
 */
#if TMR4_EN != 0
void __ISR(_TIMER_4_VECTOR, IPL_NAME(TMR4_PRIO)) isr_timer4 (void)
{
    TIMER4_IF = 0;
    
    if(m_dsc[HW_TMR4].cb != NULL)
    {
        m_dsc[HW_TMR4].cb();
    }
}
#endif

/**
 * 
 */
#if TMR5_EN != 0
void __ISR(_TIMER_5_VECTOR, IPL_NAME(TMR5_PRIO)) isr_timer5 (void)
{
    TIMER5_IF = 0;
    
    if(m_dsc[HW_TMR5].cb != NULL)
    {
        m_dsc[HW_TMR5].cb();
    }
}
#endif

/**
 * 
 */
#if TMR6_EN != 0
void __ISR(_TIMER_6_VECTOR, IPL_NAME(TMR6_PRIO)) isr_timer6 (void)
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
 * 
 * @param id
 * @return 
 */
static hw_res_t psp_tmr_id_test(tmr_t id)
{
    hw_res_t res = HW_RES_OK;
    
    //Valid id?
    if(id >= HW_TMR_NUM)
    {
        res = HW_RES_NOT_EX;
    }
    else if (reg_map[id].TxCON == NULL)
    {
        res = HW_RES_NOT_EX;
    }
    
    return res;
}

#endif
