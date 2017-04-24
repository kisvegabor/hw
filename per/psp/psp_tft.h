/**
 * @file psp_tft.h
 * 
 */

#ifndef PSP_TFT_H
#define PSP_TFT_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"

#if USE_TFT != 0
#include "hw/hw.h"
#include <stdint.h>
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
hw_res_t psp_tft_init(void);
void psp_tft_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, color_t color);
void psp_tft_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const color_t * color_p);

/**********************
 *      MACROS
 **********************/

#endif

#endif
