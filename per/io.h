/**
 * @file gpio.h
 * 
 */

#ifndef IO_H
#define IO_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"

#if USE_IO != 0
#include "hw/hw.h"
#include <stdint.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef enum
{
    IO_PORTA = 0,      IO_PORT1 = 0,
    IO_PORTB = 1,      IO_PORT2 = 1,
    IO_PORTC = 2,      IO_PORT3 = 2,
    IO_PORTD = 3,      IO_PORT4 = 3,
    IO_PORTE = 4,      IO_PORT5 = 4,
    IO_PORTF = 5,      IO_PORT6 = 5,
    IO_PORTG = 6,      IO_PORT7 = 6,
    IO_PORTH = 7,      IO_PORT8 = 7,
    IO_PORT_NUM = 8,
    IO_PORTX = 0xFF /*PORTX means invalid port*/
}io_port_t;

typedef enum
{
    IO_PIN0 = 0,   IO_PIN1,   IO_PIN2,   IO_PIN3,   
    IO_PIN4,       IO_PIN5,   IO_PIN6,   IO_PIN7,
    IO_PIN8,       IO_PIN9,   IO_PIN10,  IO_PIN11,
    IO_PIN12,      IO_PIN13,  IO_PIN14,  IO_PIN15,
    IO_PIN16,      IO_PIN17,  IO_PIN18,  IO_PIN19,
    IO_PIN20,      IO_PIN21,  IO_PIN22,  IO_PIN23,
    IO_PIN24,      IO_PIN25,  IO_PIN126, IO_PIN27,  
    IO_PIN28,      IO_PIN29,  IO_PIN30,  IO_PIN31,
    IO_PIN_NUM,
    IO_PINX = 0xFF
}io_pin_t;

typedef enum
{
    IO_DIR_IN = 0,
    IO_DIR_OUT
}io_dir_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void io_init(void);
void io_set_pin_dir(io_port_t port, io_pin_t pin,  io_dir_t dir);
void io_set_pin(io_port_t port, io_pin_t pin, uint8_t state);
uint8_t io_get_pin(io_port_t port, io_pin_t pin);
void io_set_port_dir(io_port_t port, io_dir_t dir);
void io_set_port(io_port_t port, uint32_t value);
uint32_t io_get_port(io_port_t port);

/**********************
 *      MACROS
 **********************/


#endif

#endif
