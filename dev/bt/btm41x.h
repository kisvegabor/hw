/**
 * @file btm41x.h
 * 
 */

#ifndef BTM41X_H
#define BTM41X_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_BTM41X != 0

#include "hw/hw.h"
#include <stdint.h>
#include <stdbool.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum
{
    BTM41X_STATE_INIT_ERR,
    BTM41X_STATE_NOT_CONNECTED,
    BTM41X_STATE_CONNECTED,
    BTM41X_STATE_INV = 0xFF,
}btm41x_state_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
hw_res_t btm41x_init(void);
btm41x_state_t btm41x_state(void);
hw_res_t btm41x_rec(void * buf, uint32_t * len);
hw_res_t btm41x_send(void * buf, int32_t len);

/**********************
 *      MACROS
 **********************/

#endif /*USE_BTM41x != 0*/

#endif