/**
 * @file serial.h
 * 
 */

#ifndef SERIAL_H
#define SERIAL_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"

#if USE_SERIAL != 0
#include <stdint.h>
#include "hw/hw.h"
#include "psp/psp_serial.h"

/*********************
 *      DEFINES
 *********************/
#define SERIAL_SEND_STRING  (-1)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void serial_init(void);
hw_res_t serial_send(serial_t id, const void * tx_buf, int32_t * length);
hw_res_t serial_send_force(serial_t id, const void * tx_buf, int32_t length);
hw_res_t serial_rec(serial_t id, void * rx_buf, uint32_t * length);
hw_res_t serial_rec_force(serial_t id, void * rx_buf, uint32_t length);
hw_res_t serial_set_baud(serial_t id, uint32_t baud);
hw_res_t serial_clear_rx_buf(serial_t id) ;
uint32_t serial_get_send_time(uint32_t byte_num, uint32_t baud);


/**********************
 *      MACROS
 **********************/

#endif

#endif
