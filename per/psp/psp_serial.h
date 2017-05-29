/**
 * @file psp_serial.h
 * 
 */

#ifndef PSP_SERIAL_H
#define PSP_SERIAL_H

/*********************
 *      INCLUDES
 *********************/
#include "hw/hw.h"

#if USE_SERIAL != 0
#include <stdint.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef enum
{
    HW_SERIAL1 = 0,
    HW_SERIAL2,
    HW_SERIAL3,
    HW_SERIAL4,
    HW_SERIAL5,
    HW_SERIAL6,
    HW_SERIAL_NUM,
    HW_SERIALX = 0xFF /*SERIALX means invalid/unused module*/
}serial_t;

typedef enum
{
    SERIAL_MODE_BASIC =    0,  
    SERIAL_MODE_RTS_EN =   1 << 0,
    SERIAL_MODE_CTS_EN =   1 << 1,
    SERIAL_MODE_PAR_ODD =  1 << 2,
    SERIAL_MODE_PAR_EVEN = 1 << 3,
    SERIAL_MODE_2_STOP =   1 << 4,
    SERIAL_MODE_TX_INV =   1 << 5,
    SERIAL_MODE_RX_INV =   1 << 6
}serial_mode_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void psp_serial_init(void);
hw_res_t psp_serial_wr(serial_t id, uint8_t tx);
hw_res_t psp_serial_rd(serial_t id, uint8_t * rx);
hw_res_t psp_serial_set_baud(serial_t id, uint32_t baud);
hw_res_t psp_serial_clear_rx_buf(serial_t id);

/**********************
 *      MACROS
 **********************/

#endif
#endif
