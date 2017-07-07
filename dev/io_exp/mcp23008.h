/**
 * @file mcp23008.h
 * 
 */

#ifndef MCP23008_H
#define MCP23008_H

#ifdef __cplusplus
extern "C" {
#endif


/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"

#if USE_MCP23008 != 0    
#include <stdint.h>
    
    
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void mcp23008_init(void);
void mcp23008_pin_out(uint8_t pin);
void mcp23008_pin_in(uint8_t pin);
void mcp23008_pin_set(uint8_t pin);
void mcp23008_pin_clr(uint8_t pin);
uint8_t mcp23008_pin_get(uint8_t pin);

/**********************
 *      MACROS
 **********************/

#endif      /*USE_MCP23008 != 0*/
    
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
