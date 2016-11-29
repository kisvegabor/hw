/**
 * @file rdisp.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_RDISP != 0

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "hw/per/spi.h"
#include "hw/per/serial.h"
#include "hw/per/io.h"
#include "hw/per/tick.h"
#include "rdisp.h"
#include "misc/others/slip.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct
{
    uint16_t x :5;
    uint16_t y :5;
    uint16_t intense :6;
}rdisp_packet_t;


/**********************
 *  STATIC PROTOTYPES
 **********************/
static void rdisp_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2);

/**********************
 *  STATIC VARIABLES
 **********************/
static uint8_t disp_fb[RDISP_HOR_RES * RDISP_VER_RES];
static int32_t last_x1;
static int32_t last_y1;
static int32_t last_x2;
static int32_t last_y2;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialization for the remote display
 */
void rdisp_init(void)
{
    serial_set_baud(RDISP_DRV, RDISP_BAUD);
    
    memset(disp_fb, 0x00, sizeof(disp_fb));
}

/**
 * Mark out a rectangle
 * @param x1 left coordinate of the rectangle
 * @param y1 top coordinate of the rectangle
 * @param x2 right coordinate of the rectangle
 * @param y2 bottom coordinate of the rectangle
 */
void rdisp_set_area(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
	last_x1 = x1;
	last_y1 = y1;
	last_x2 = x2;
	last_y2 = y2;
}

/**
 * Fill the previously marked area with a color
 * @param color fill color
 */
void rdisp_fill(color_t color) 
{
     /*Return if the area is out the screen*/
    if(last_x2 < 0) return;
    if(last_y2 < 0) return;
    if(last_x1 > RDISP_HOR_RES - 1) return;
    if(last_y1 > RDISP_VER_RES - 1) return;

    /*Truncate the area to the screen*/
    int32_t act_x1 = last_x1 < 0 ? 0 : last_x1;
    int32_t act_y1 = last_y1 < 0 ? 0 : last_y1;
    int32_t act_x2 = last_x2 > RDISP_HOR_RES - 1 ? RDISP_HOR_RES - 1 : last_x2;
    int32_t act_y2 = last_y2 > RDISP_VER_RES - 1 ? RDISP_VER_RES - 1 : last_y2;
    
    int32_t x, y;
    uint8_t bright = color_brightness(color);
    
    /*Refresh frame buffer*/
    for(y= act_y1; y <= act_y2; y++) {
        for(x = act_x1; x <= act_x2; x++) {
                disp_fb[x+ y * RDISP_HOR_RES] = bright;
        }
    }
    
    rdisp_flush(act_x1, act_y1, act_x2, act_y2);
}

/**
 * Put a pixel map to the previously marked area
 * @param color_p an array of pixels
 */
void rdisp_map(color_t * color_p) 
{
     /*Return if the area is out the screen*/
    if(last_x2 < 0) return;
    if(last_y2 < 0) return;
    if(last_x1 > RDISP_HOR_RES - 1) return;
    if(last_y1 > RDISP_VER_RES - 1) return;

    /*Truncate the area to the screen*/
    int32_t act_x1 = last_x1 < 0 ? 0 : last_x1;
    int32_t act_y1 = last_y1 < 0 ? 0 : last_y1;
    int32_t act_x2 = last_x2 > RDISP_HOR_RES - 1 ? RDISP_HOR_RES - 1 : last_x2;
    int32_t act_y2 = last_y2 > RDISP_VER_RES - 1 ? RDISP_VER_RES - 1 : last_y2;
    
    int32_t x, y;
    
    /*Refresh frame buffer*/
    for(y= act_y1; y <= act_y2; y++) {
        for(x = act_x1; x <= act_x2; x++) {
            disp_fb[x + y * RDISP_HOR_RES] = color_brightness(*color_p);
            color_p ++;
        }
        color_p += last_x2 - act_x2; /*Next row*/
    }
    
    rdisp_flush(act_x1, act_y1, act_x2, act_y2);
}
/**********************
 *   STATIC FUNCTIONS
 **********************/
/**
 * Flush a specific part of the buffer to the display
 * @param x1 left coordinate of the area to flush
 * @param y1 top coordinate of the area to flush
 * @param x2 right coordinate of the area to flush
 * @param y2 bottom coordinate of the area to flush
 */
static void rdisp_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    int32_t x, y;
    rdisp_packet_t pack;
    uint8_t slip_buf[2 * sizeof(rdisp_packet_t) + 1];
    int32_t slip_len;
    
      
//    pack.x = 2;
//    pack.y = 3;
//    pack.intense = 0x16;
//       
//    slip_len = slip_encode(slip_buf, &pack, sizeof(rdisp_packet_t));
//    serial_send_force(RDISP_DRV, slip_buf, slip_len);
//
//    return;
    
    
    for(y = y1; y <= y2; y++) {
        for(x = x1; x <= x2; x++) {
            pack.x = x;
            pack.y = y;
            pack.intense = disp_fb[x+ y * RDISP_HOR_RES] >> 2;
            
            slip_len = slip_encode(slip_buf, &pack, sizeof(rdisp_packet_t));
            serial_send_force(RDISP_DRV, slip_buf, slip_len);
        }
    }
}

#endif