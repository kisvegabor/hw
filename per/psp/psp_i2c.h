/**
 * @file psp_i2c.h
 * 
 */

#ifndef PSP_I2C_H
#define PSP_I2C_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_I2C != 0

#include <stdint.h>
#include <stdbool.h>
#include "hw/hw.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum
{
    HW_I2C1 = 0,
    HW_I2C2,
    HW_I2C_NUM,
    HW_I2CX = 0xFF /*always ignored*/
}i2c_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void psp_i2c_init(void);
hw_res_t psp_i2c_start(i2c_t id);
hw_res_t psp_i2c_restart(i2c_t id);
hw_res_t psp_i2c_stop(i2c_t id);
hw_res_t psp_i2c_wr(i2c_t id, uint8_t data);
hw_res_t psp_i2c_rd(i2c_t id, uint8_t * data, bool ack);


/**********************
 *      MACROS
 **********************/

#endif

#endif
