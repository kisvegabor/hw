/**
 * @file tft.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_TFT != 0

#include "psp/psp_tft.h"

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
 * Initialize the TFT
 */
hw_res_t tft_init(void)
{
	hw_res_t res  = HW_RES_OK;

	psp_tft_init();

    return res;
}

/**
 * Mark out an area on the TFT
 * @param x1 left coordinate
 * @param y1 top coordinate
 * @param x2 right coordinate
 * @param y2 bottom coordinate
 */
void tft_set_area(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
	psp_tft_set_area(x1, y1, x2, y2);
}

/**
 * Fill out the marked area with a color
 * @param color fill color
 */
void tft_fill(color_t color)
{
	psp_tft_fill(color);
}

/** 
 * Put a color map to the marked area
 * @param color_p an array of colors
 */
void tft_map(color_t * color_p)
{
	psp_tft_map(color_p);
}



/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
