/**
 * @file led.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"

#if USE_LED != 0

#include "led.h"
#include "hw/per/io.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct
{
    io_port_t port;
    io_pin_t pin;
}led_io_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static const led_io_t led_io[] = 
{
    {LED1_PORT, LED1_PIN},
    {LED2_PORT, LED2_PIN},
    {LED3_PORT, LED3_PIN},
    {LED4_PORT, LED4_PIN},
    {LED5_PORT, LED5_PIN},
    {LED6_PORT, LED6_PIN},
    {LED7_PORT, LED7_PIN},
    {LED8_PORT, LED8_PIN},
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the LEDs
 */
void led_init(void)
{
    uint8_t i;
    for(i = LED1; i < LED_NUM; i++) {
        io_set_pin_dir(led_io[i].port, led_io[i].pin, IO_DIR_OUT);
        io_set_pin(led_io[i].port, led_io[i].pin, 0);
    }
}

/**
 * Turn on a LED
 * @param led a LED id from led_t enum
 * @return HW_RES_OK or any error from hw_res_t
 */
hw_res_t led_on(led_t led)
{
    hw_res_t res = HW_RES_OK;
    
    if(led < LED_NUM)
    {
        io_set_pin(led_io[led].port, led_io[led].pin, 1);
    } else {
        res = HW_RES_INV_PARAM;
    }
    
    return res;
}

/**
 * Turn off a LED
 * @param led a LED id from led_t enum
 * @return HW_RES_OK or any error from hw_res_t
 */
hw_res_t led_off(led_t led)
{
    hw_res_t res = HW_RES_OK;
    
    if(led < LED_NUM)
    {
        io_set_pin(led_io[led].port, led_io[led].pin, 0);
    }
    else
    {
        res = HW_RES_INV_PARAM;
    }
    
    return res;
}

/**
 * Toggle a LED
 * @param led a LED id from led_t enum
 * @return HW_RES_OK or any error from hw_res_t
 */
hw_res_t led_toggle(led_t led)
{
    hw_res_t res = HW_RES_OK;
    
    if(led < LED_NUM) {
        if(io_get_pin(led_io[led].port, led_io[led].pin) == 0) {
            io_set_pin(led_io[led].port, led_io[led].pin, 1);
        } else {
            io_set_pin(led_io[led].port, led_io[led].pin, 0);
        }
    } else {
        res = HW_RES_INV_PARAM;
    }
    
    return res;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
