/**
 * @file SSD1963.h
 * 
 */

#ifndef SSD1963_H
#define SSD1963_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_SSD1963 != 0

#include <stdint.h>
#include "hw/hw.h"
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
void ssd1963_init(void);
void ssd1963_set_area(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
void ssd1963_fill(color_t  color);
void ssd1963_map(color_t * color_p);

/**********************
 *      MACROS
 **********************/

#endif

#endif
