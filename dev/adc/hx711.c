/**
 * @file hx711.c
 * 24 bit ADC on 2 wire
 */

#include "hx711.h"
#if USE_HX711 != 0

#include "hw/per/io.h"
#include "hw/per/tick.h"


/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/
#define HX711_PERIOD_HALF  10   /*Time of clock '1' or '0' level [us]*/

/**********************
 *      TYPEDEFS
 **********************/

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
 * Initialize the HX711 AD converter
 */
void hx711_init(void)
{
    io_set_pin_dir(HX711_SCK_PORT, HX711_SCK_PIN, IO_DIR_OUT);
    io_set_pin_dir(HX711_SDO_PORT, HX711_SDO_PORT, IO_DIR_IN);
    io_set_pin(HX711_SCK_PORT, HX711_SCK_PIN, 0);
}

/**
 * Read the ADC
 * @return 24 bit value
 */
uint32_t hx711_meas(void)
{
    uint32_t c;
    uint32_t i;
    
    io_set_pin(HX711_SCK_PORT, HX711_SCK_PIN, 0);
    
    while(io_get_pin(HX711_SDO_PORT, HX711_SDO_PIN)); /*Be sure the prev. cycle is finished*/
    
    tick_wait_us(HX711_PERIOD_HALF);                    /*Wait to be ready*/
    c = 0;
    for(i = 0; i < 24; i ++){
        io_set_pin(HX711_SCK_PORT, HX711_SCK_PIN, 1);
        tick_wait_us(HX711_PERIOD_HALF);
        c = c << 1;
        io_set_pin(HX711_SCK_PORT, HX711_SCK_PIN, 0);
        tick_wait_us(HX711_PERIOD_HALF);
        if(io_get_pin(HX711_SDO_PORT, HX711_SDO_PIN)) c ++;
    }
    
    io_set_pin(HX711_SCK_PORT, HX711_SCK_PIN, 1);
    tick_wait_us(HX711_PERIOD_HALF);
    c = c ^ 0x800000;
    io_set_pin(HX711_SCK_PORT, HX711_SCK_PIN, 0);
    
    return(c); 
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*USE_HX711*/
