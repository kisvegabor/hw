/**
 * @file R61581.h
 * 
 */

#ifndef R61581_H
#define R61581_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_R61581 != 0

#include <stdint.h>
#include "hw/hw.h"
#include "misc/gfx/color.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void r61581_init(void);
void r61581_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, color_t color);
void r61581_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, color_t * color_p);
/**********************
 *      MACROS
 **********************/

#endif

#endif
