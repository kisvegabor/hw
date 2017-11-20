/**
 * @file ds18b20.h
 * 
 */

#ifndef DS18B20_H
#define DS18B20_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_DS18B20 != 0
    
#include <stdint.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    DS18B20_BUS_0,
    DS18B20_BUS_1,
    DS18B20_BUS_2,
    DS18B20_BUS_3,
}ds18b20_bus_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void ds18b20_init(void);
void ds18b20_start_conv(ds18b20_bus_t bus);
int16_t ds18b20_read_result(ds18b20_bus_t bus);

/**********************
 *      MACROS
 **********************/

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DS18B20_H*/
