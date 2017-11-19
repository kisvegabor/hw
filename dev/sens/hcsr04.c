/**
 * @file hcsr04.c
 * Ultrasonic module
 */

/*********************
 *      INCLUDES
 *********************/
#include "hcsr04.h"
#include "hw/per/io.h"
#include "hw/per/tick.h"

#if USE_HCSR04 != 0

/*********************
 *      DEFINES
 *********************/
#define HCSR04_US_TO_CM     58          /* Echo length in us which means 1 cm */
#define HCSR04_DISTANCE_MAX  100         /* [cm] */
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
 * initialize the HC-SR04 ultrasonic proximity measure module
 */
void hcsr04_init(void) {
    io_set_pin_dir(HCSR04_TRIG_PORT, HCSR04_TRIG_PIN, IO_DIR_OUT);
    io_set_pin(HCSR04_TRIG_PORT, HCSR04_TRIG_PIN, 0);
    io_set_pin_dir(HCSR04_ECHO_PORT, HCSR04_ECHO_PIN, IO_DIR_IN);
}

/**
 * Measure distance
 * @return Distance of an object im cm
 */
uint16_t hcsr04_meas(void) {
    io_set_pin(HCSR04_TRIG_PORT, HCSR04_TRIG_PIN, 1);
    tick_wait_us(20);
    io_set_pin(HCSR04_TRIG_PORT, HCSR04_TRIG_PIN, 0);

    uint32_t to = 0;
    while(io_get_pin(HCSR04_ECHO_PORT, HCSR04_ECHO_PIN) == 0){
        tick_wait_us(10);
        to++;
        if(to > 100) return HCSR04_DISTANCE_MAX;
    }
    
    uint16_t c = 0;
    do {
        tick_wait_us(HCSR04_US_TO_CM);
        c++;
        if (c > HCSR04_DISTANCE_MAX) break;
    } while (io_get_pin(HCSR04_ECHO_PORT, HCSR04_ECHO_PIN));

    return c;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif