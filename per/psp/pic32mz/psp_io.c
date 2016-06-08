/**
 * @file gpio.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"

#if USE_IO != 0 && PSP_PIC32MZ != 0

#include <xc.h>

#include "hw/hw.h"
#include "hw/per/io.h"
#include "hw/per/psp/psp_io.h"
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
 * Initialize the IO ports
 */
void psp_io_init(void)
{ 
    /*Enable digital buffers by default*/
#ifdef _ANSELA_w_MASK
    ANSELA = 0;
#endif
    
#ifdef _ANSELB_w_MASK
    ANSELB = 0;
#endif
    
#ifdef _ANSELC_w_MASK
    ANSELC = 0;
#endif
    
#ifdef _ANSELD_w_MASK
    ANSELD = 0;
#endif
    
#ifdef _ANSELE_w_MASK
    ANSELE = 0;
#endif
    
#ifdef _ANSELF_w_MASK
    ANSELF = 0;
#endif
    
#ifdef _ANSELG_w_MASK
    ANSELG = 0;
#endif
    
#ifdef _ANSELA_w_MASK
    ANSELA = 0;
#endif
    
#ifdef _ANSELH_w_MASK
    ANSELH = 0;
#endif
    
}

/**
 * Read a port
 * @param port id of a port from io_port_t enum
 * @return the value of the port
 */
volatile unsigned int psp_io_rd_port(io_port_t port)
{
    if(reg_map[port].PORTx != NULL) return  *reg_map[port].PORTx;
    else  return 0;
}

/**
 * Write a port
 * @param port id of port from io_port_t
 * @param value value to write
 */
void psp_io_wr_port(io_port_t port, volatile unsigned int value)
{
    if(reg_map[port].LATx != NULL) (*reg_map[port].LATx) = value;
}

/**
 * Read the direction register of a port
 * @param port d of port from io_port_t
 * @return the value of the direction register of a port
 */
volatile unsigned int psp_io_rd_dir(io_port_t port)
{
    volatile unsigned int tmp = 0;
    if(reg_map[port].TRISx != NULL) tmp = *reg_map[port].TRISx;
    
    /*Invert the value if it is necessary*/
    if (IO_DIR_OUT != 0)  tmp = ~tmp;
    
    return tmp;
}

/**
 * Write the direction register of a port
 * @param port id of port from io_port_t
 * @param value value to write
 */
void psp_io_wr_dir(io_port_t port, volatile unsigned int value)
{
    /*Invert the value if it is necessary*/
    if (IO_DIR_OUT != 0) value = ~value;
    
    if(reg_map[port].TRISx != NULL)  (*reg_map[port].TRISx) = value;
}


/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
