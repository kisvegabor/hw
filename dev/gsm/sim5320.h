/**
 * @file sim5320.h
 * 
 */

#ifndef SIM5320_H
#define SIM5320_H

#ifdef __cplusplus
extern "C" {
#endif


/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_SIM5320 != 0

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
    SIM5320_STATE_OK = 0,
    SIM5320_STATE_BUSY,
    SIM5320_STATE_ERROR,
    SIM5320_STATE_NUM,
    SIM5320_STATE_INV = 0xFF
}sim5320_state_t;

typedef void (*sim5320_cb_t)(sim5320_state_t, const char *); 
/**********************
 * GLOBAL PROTOTYPES
 **********************/
    
void sim5320_init(void);
sim5320_state_t sim5320_netw_con(const char * apn, sim5320_cb_t cb);
sim5320_state_t sim5320_netw_leave(sim5320_cb_t cb);
sim5320_state_t sim5320_tcp_con(const char * ip, const char * port, sim5320_cb_t cb);
sim5320_state_t sim5320_tcp_leave(sim5320_cb_t cb);
sim5320_state_t sim5320_tcp_transf(const void * data, uint16_t len, sim5320_cb_t cb);
bool sim5320_busy(void);

/**********************
 *      MACROS
 **********************/

#endif  /*USE_SIM5320 != 0*/

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif