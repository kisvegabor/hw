/**
 * @file buzzer.h
 * 
 */

#ifndef BUZZER_H
#define BUZZER_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_BUZZER != 0

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

void buzzer_init(void);
void buzzer_on(void);
void buzzer_off(void);
void buzzer_toggle(void);
void buzzer_beep(void);

#if BUZZER_EXT_DRIVE != 0
void buzzer_play(const * uint8_t pattern_p);
void buzzer_stop(void);
#endif

/**********************
 *      MACROS
 **********************/

#endif /*USE_BUZZER*/

#endif