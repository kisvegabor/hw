/**
 * @file buzzer.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_BUZZER != 0

#include "buzzer.h"
#include "misc/os/tick.h"
#include "hw/per/io.h"


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

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the buzzer
 */
void buzzer_init(void)
{
    io_set_pin_dir(BUZZER_PORT, BUZZER_PIN, IO_DIR_OUT);
    
    buzzer_off();
}

/**
 * Turn on the buzzer
 */
void buzzer_on(void)
{
#if BUZZER_INV == 0
    io_set_pin(BUZZER_PORT, BUZZER_PIN, 1);
#else
    io_set_pin(BUZZER_PORT, BUZZER_PIN, 0);
#endif
}

/**
 * Turn off the buzzer
 */
void buzzer_off(void)
{
#if BUZZER_INV == 0
    io_set_pin(BUZZER_PORT, BUZZER_PIN, 0);
#else
    io_set_pin(BUZZER_PORT, BUZZER_PIN, 1);
#endif
}

/**
 * Toggle the state of the buzzer
 */
void buzzer_toggle(void)
{
    if(io_get_pin(BUZZER_PORT, BUZZER_PIN) == 0) io_set_pin(BUZZER_PORT, BUZZER_PIN, 1);
    else io_set_pin(BUZZER_PORT, BUZZER_PIN, 0);
}

/**
 * Make a beep with the buzzer. (Blocking wait)
 */
void buzzer_beep(void)
{
    buzzer_on();
    tick_wait_ms(BUZZER_BEEP_ON_TIME);
    buzzer_off();
    tick_wait_ms(BUZZER_BEEP_OFF_TIME);
}

#if BUZZER_EXT_DRIVE != 0
void buzzer_play(const * uint8_t pattern_p)
{
    
}

void buzzer_stop(void)
{
    
}

#endif /*BUZZER_EXT_DRIVE*/
/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*USE_BUZZER*/