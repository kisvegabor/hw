/**
 * @file ST7565.h
 * 
 */

#ifndef ST7565_H
#define ST7565_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_ST7565 != 0
#include <stdint.h>
#include "misc/others/color.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void st7565_init(void);
void st7565_set_area(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
void st7565_fill(color_t color);
void st7565_map(color_t * color_p);

/**********************
 *      MACROS
 **********************/

#endif

#endif
