/**
 * @file pcf8574.c
 * 
 */
#include "pcf8574.h"

#if USE_PCF8574 != 0

#include <stddef.h>
#include "hw/per/i2c.h"

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static uint8_t out;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void pcf8574_init(void)
{
    /*All pins high by default. Weak current sources*/
    out = 0xff;
    i2c_send(PCF8574_DRV, PCF8574_ADR, &out, sizeof(out));
}

void pcf8574_pin_set(uint8_t pin)
{
    out |= 1 << pin;
    i2c_send(PCF8574_DRV, PCF8574_ADR, &out, sizeof(out));
}

void pcf8574_pin_clr(uint8_t pin)
{
    out &= ~(1 << pin);
    i2c_send(PCF8574_DRV, PCF8574_ADR, &out, sizeof(out));
}


uint8_t pcf8574_pin_get(uint8_t pin)
{
    char buf; 
    i2c_read(PCF8574_DRV, PCF8574_ADR, NULL, 0, &buf, sizeof(buf));
    
    return (buf & (1 << pin)) == 0 ? 0 : 1;
    
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /* USE_PCF8574 != 0*/
