/**
 * @file mcp23008.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "mcp23008.h"

#if USE_MCP23008 != 0

#include "hw/per/i2c.h"

/*********************
 *      DEFINES
 *********************/
#define MCP23008_REG_DIR    0x00
#define MCP23008_REG_GPIO   0x09

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static uint8_t dir = 0xff;
static uint8_t out = 0;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void mcp23008_init(void)
{
    
}

void mcp23008_pin_out(uint8_t pin)
{
    dir &= ~(1 << pin);
    char buf[2];
    buf[0] = MCP23008_REG_DIR;
    buf[1] = dir; 
    i2c_send(MCP23008_DRV, MCP23008_ADR, buf, sizeof(buf));  
}


void mcp23008_pin_in(uint8_t pin)
{
    dir |= 1 << pin;
    char buf[2];
    buf[0] = MCP23008_REG_DIR;
    buf[1] = dir; 
    i2c_send(MCP23008_DRV, MCP23008_ADR, buf, sizeof(buf));
}

void mcp23008_pin_set(uint8_t pin)
{
    out |= 1 << pin;
    char buf[2];
    buf[0] = MCP23008_REG_GPIO;
    buf[1] = out; 
    i2c_send(MCP23008_DRV, MCP23008_ADR, buf, sizeof(buf));
}

void mcp23008_pin_clr(uint8_t pin)
{
    out &= ~(1 << pin);
    char buf[2];
    buf[0] = MCP23008_REG_GPIO;
    buf[1] = out; 
    i2c_send(MCP23008_DRV, MCP23008_ADR, buf, sizeof(buf));  
}

uint8_t mcp23008_pin_get(uint8_t pin)
{
    char cmd = MCP23008_REG_GPIO;
    char buf; 
    i2c_read(MCP23008_DRV, MCP23008_ADR, &cmd, sizeof(cmd), &buf, sizeof(buf));  
    
    return (buf & (1 << pin)) == 0 ? 0 : 1;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*USE_MCP23008 != 0*/