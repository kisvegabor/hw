/**
 * @file io.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_IO != 0

#include "hw/hw.h"
#include "io.h"
#include "psp/psp_io.h"
#include <stddef.h>
#include <string.h>

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

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize to IO ports
 */
void io_init(void)
{ 
    psp_io_init();
}

/**
 * Set the direction of a pin
 * @param port an io port from io_port_t enum
 * @param pin a pin from io_pin_t enum
 * @param dir IO_DIR_IN or IO_DIR_IN
 */
void io_set_pin_dir(io_port_t port, io_pin_t pin,  io_dir_t dir)
{
    if(port != IO_PORTX && pin != IO_PINX) {
        volatile unsigned int dir_reg;
        dir_reg = psp_io_rd_dir(port);

        dir_reg &= (~(1 << pin)); /*Clear the dir bit*/
        dir_reg |= (dir << pin);  /*Set the dir bit*/
            
        psp_io_wr_dir(port, dir_reg);
        
    }
}

/**
 * Set or clear a pin
 * @param port an io port from io_port_t enum
 * @param pin a pin from io_pin_t enum
 * @param state 1 or 0
 */
void io_set_pin(io_port_t port, io_pin_t pin, uint8_t state)
{    
    if(port != IO_PORTX && pin != IO_PINX) {
        volatile unsigned int port_reg;
        port_reg = psp_io_rd_port(port);

        if(state == 0) port_reg &= (~(1 << pin));
        else port_reg |= (1 << pin);
        
        psp_io_wr_port(port, port_reg);
    } 

}

/**
 * Get the state of a pin
 * @param port an io port from io_port_t enum
 * @param pin a pin from io_pin_t enum
 * @return state the state of a pin (1 or 0)
 */
uint8_t io_get_pin(io_port_t port, io_pin_t pin)
{
    if(port != IO_PORTX && pin != IO_PINX) {
        volatile unsigned int port_reg;
        port_reg = psp_io_rd_port(port);
        
        if(port_reg & (1 << pin)) return 1;
        else return 0;
    }
    
    return 0;
}

/**
 * Set the direction a port
 * @param port port an io port from io_port_t enum
 * @param dir IO_DIR_IN or IO_DIR_IN
 */
void io_set_port_dir(io_port_t port, io_dir_t dir)
{
    if(port != IO_PORTX) {
        /*Create the value to set*/
        volatile unsigned int dir_val;
        
        if(dir == IO_DIR_IN) {
            if(IO_DIR_IN == 0)  dir_val = 0;
            else  memset((unsigned int *)&dir_val, 0xFF, sizeof(dir_val));
        } else {
            if(IO_DIR_OUT == 0) dir_val = 0; 
            else  memset((unsigned int *)&dir_val, 0xFF, sizeof(dir_val));
        }
            
        psp_io_wr_dir(port, dir_val);
    }
}


/**
 * Seta  value to a port
 * @param port port an io port from io_port_t enum
 * @param value the value to set
 */
void io_set_port(io_port_t port, uint32_t value)
{
    if(port != IO_PORTX) psp_io_wr_port(port, (volatile unsigned int) value);
}

/**
 * Read a port
 * @param port port an io port from io_port_t enum
 * @return the state of the port
 */
uint32_t io_get_port(io_port_t port)
{
    return psp_io_rd_port(port);   
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
#endif
