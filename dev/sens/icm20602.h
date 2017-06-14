/**
 * @file icm20602.h
 *
 */

#ifndef ICM20602_H
#define ICM20602_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"

#if USE_ICM20602 != 0

#include <stdint.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct
{
	int16_t acc_x;
	int16_t acc_y;
	int16_t acc_z;
	int16_t temp;
	int16_t gyro_x;
	int16_t gyro_y;
	int16_t gyro_z;
}icm20602_data_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void icm20602_init(void);
void icm20602_read(icm20602_data_t * buf);

/**********************
 *      MACROS
 **********************/

#endif /*USE_ICM20602 != 0*/

#endif

