/**
 * @file gpio.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_IO != 0 && PSP_PIC24F_33F != 0

#include "hw/hw.h"
#include "hw/per/io.h"
#include "hw/per/psp/psp_io.h"
#include <stddef.h>
#include <xc.h>

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
reg_map_t reg_map[] = 
{
            /*TRIS*/    /*PORT*/    /*LAT*/
#ifdef TRISA
           {&TRISA,     &PORTA,      &LATA},             
#else
           {NULL,       NULL,       NULL}, 
#endif
#ifdef TRISB
           {&TRISB,     &PORTB,      &LATB},          
#else
           {NULL,       NULL,       NULL}, 
#endif

#ifdef TRISC
           {&TRISC,     &PORTC,      &LATC},             
#else
           {NULL,       NULL,       NULL}, 
#endif
#ifdef TRISD 
           {&TRISD,     &PORTD,      &LATD},             
#else
           {NULL,       NULL,       NULL}, 
#endif
#ifdef TRISE
           {&TRISE,     &PORTE,      &LATE},             
#else
           {NULL,       NULL,       NULL}, 
#endif
#ifdef TRISF
           {&TRISF,     &PORTF,      &LATF},             
#else
           {NULL,       NULL,       NULL}, 
#endif
#ifdef TRISG
           {&TRISG,     &PORTG,      &LATG},             
#else
           {NULL,       NULL,       NULL}, 
#endif

#ifdef TRISH
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
#ifdef ANSA
    ANSA = 0;
#endif
    
#ifdef ANSELA
    ANSELA = 0;
#endif

#ifdef ANSB
    ANSB = 0;
#endif
    
#ifdef ANSELB
    ANSELB = 0;
#endif


#ifdef ANSC
    ANSC = 0;
#endif
    
#ifdef ANSELC
    ANSELC = 0;
#endif

#ifdef ANSD
    ANSD = 0;
#endif

#ifdef ANSELD
    ANSELD = 0;
#endif
    
#ifdef ANSE
    ANSE = 0;
#endif
    
#ifdef ANSELE
    ANSELE = 0;
#endif
    
#ifdef ANSF
    ANSF = 0;
#endif
    
#ifdef ANSELF
    ANSELF = 0;
#endif
    
#ifdef ANSG
    ANSG = 0;
#endif
    
#ifdef ANSELG
    ANSELG = 0;
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
    else return 0;
}

/**
 * Write a port
 * @param port id of port from io_port_t
 * @param value value to write
 */
void psp_io_wr_port(io_port_t port, volatile unsigned int value)
{   
    if(reg_map[port].LATx != NULL)  (*reg_map[port].LATx) = value;
}

/**
 * Read the direction register of a port
 * @param port d of port from io_port_t
 * @return the value of the direction register of a port
 */
volatile unsigned int psp_io_rd_dir(io_port_t port)
{   
    volatile unsigned int tmp = 0;
    if(reg_map[port].TRISx != NULL)  tmp =  *reg_map[port].TRISx;
    
    /*Invert the value if it is necessary*/
    if (IO_DIR_OUT != 0) tmp = ~(tmp);
    
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
    if(reg_map[port].TRISx != NULL) (*reg_map[port].TRISx) = value;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
#endif
