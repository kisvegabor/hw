/**
 * @file i2c.h
 * 
 */

#ifndef I2C_H
#define I2C_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_I2C != 0
#include <stdint.h>
#include "psp/psp_i2c.h"
#include "hw/hw.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void i2c_init(void);
hw_res_t i2c_send(i2c_t id, uint8_t adr, void * data_p, uint16_t len);
hw_res_t i2c_read(i2c_t id, uint8_t adr, uint8_t cmd, void * data_p, uint16_t len); 

/**********************
 *      MACROS
 **********************/

#endif

#endif
