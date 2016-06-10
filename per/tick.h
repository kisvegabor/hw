/**
 * @file sys.h
 * 
 */

#ifndef TICK_H
#define TICK_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_TICK != 0

#include <stdint.h>
#include <stdbool.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void tick_init(void);
void tick_wait_ms (uint32_t delay);
void tick_wait_us (uint32_t delay);
uint32_t tick_get(void);
uint32_t tick_elaps(uint32_t time_prev);
bool tick_add_func(void(*fp)(void));
void tick_rem_func(void(*cb)(void));

/**********************
 *      MACROS
 **********************/

#endif

#endif
