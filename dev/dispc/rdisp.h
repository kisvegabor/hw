/**
 * @file rdisp.h
 * 
 */

#ifndef RDISP_H
#define RDISP_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_RDISP != 0
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
void rdisp_init(void);
void rdisp_set_area(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
void rdisp_fill(color_t color);
void rdisp_map(color_t * color_p);

/**********************
 *      MACROS
 **********************/

#endif

#endif
