/**
 * @file gpio.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"

#if USE_IO != 0 && PSP_PIC32MX != 0

#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING
#include <plib.h>

#include "hw/hw.h"
#include "hw/per/io.h"
#include <stddef.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct
{
    volatile unsigned int * TRISx;
    volatile unsigned int * PORTx;
    volatile unsigned int * LATx;
}reg_map_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static reg_map_t reg_map[] = 
{
            /*TRIS*/    /*PORT*/    /*LAT*/
#ifdef _TRISA_w_MASK
           {&TRISA,     &PORTA,      &LATA},             
#else
           {NULL,       NULL,       NULL}, 
#endif
#ifdef _TRISB_w_MASK
           {&TRISB,     &PORTB,      &LATB},          
#else
           {NULL,       NULL,       NULL}, 
#endif

#ifdef _TRISC_w_MASK
           {&TRISC,     &PORTC,      &LATC},             
#else
           {NULL,       NULL,       NULL}, 
#endif
#ifdef _TRISD_w_MASK
           {&TRISD,     &PORTD,      &LATD},             
#else
           {NULL,       NULL,       NULL}, 
#endif
#ifdef _TRISE_w_MASK
           {&TRISE,     &PORTE,      &LATE},             
#else
           {NULL,       NULL,       NULL}, 
#endif
#ifdef _TRISF_w_MASK
           {&TRISF,     &PORTF,      &LATF},             
#else
           {NULL,       NULL,       NULL}, 
#endif
#ifdef _TRISG_w_MASK
           {&TRISG,     &PORTG,      &LATG},             
#else
           {NULL,       NULL,       NULL}, 
#endif
#ifdef _TRISH_w_MASK
           {&TRISH,     &PORTH,      &LATH},             
#else
           {NULL,       NULL,       NULL}, 
#endif

};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * 
 */
void psp_io_init(void)
{ 
    /*Enable digital buffers by default*/
    AD1PCFG = 0xFFFFFFFF;
    
    mJTAGPortEnable(0);    
}

/**
 * 
 * @param port
 * @param value
 * @return 
 */
hw_res_t psp_io_rd_port(io_port_t port, volatile unsigned int * value)
{
    hw_res_t res = HW_RES_OK;
    
    if(reg_map[port].PORTx != NULL) {
        (*value) =  *reg_map[port].PORTx;
    } else {
        res = HW_RES_NOT_EX;
    }
    
    return res;
}

/**
 * 
 * @param port
 * @param value
 * @return 
 */
hw_res_t psp_io_wr_port(io_port_t port, volatile unsigned int value)
{
    hw_res_t res = HW_RES_OK;
    
    if(reg_map[port].LATx != NULL) {
        (*reg_map[port].LATx) = value;
    } else {
        res = HW_RES_NOT_EX;
    }

    return res;
}

/**
 * 
 * @param port
 * @param value
 * @return 
 */
hw_res_t psp_io_rd_dir(io_port_t port, volatile unsigned int * value)
{
    hw_res_t res = HW_RES_OK;
    
    if(reg_map[port].TRISx != NULL) {
        (*value) =  *reg_map[port].TRISx;
    } else {
        res = HW_RES_NOT_EX;
    }
    
    /*Invert the value if it is necessary*/
    if (IO_DIR_OUT != 0) {
        *value = ~(*value);
    }
    
    return res;
}

/**
 * 
 * @param port
 * @param value
 * @return 
 */
hw_res_t psp_io_wr_dir(io_port_t port, volatile unsigned int value)
{
    hw_res_t res = HW_RES_OK;
    
    /*Invert the value if it is necessary*/
    if (IO_DIR_OUT != 0) {
        value = ~value;
    }
    
    if(reg_map[port].TRISx != NULL) {
        (*reg_map[port].TRISx) = value;
    } 
    else {
        res = HW_RES_NOT_EX;
    }

    return res;
}


/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
