/**
 * @file esp8266.h
 * 
 */

#ifndef ESP8266_H
#define ESP8266_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_ESP8266 != 0

#include <stdbool.h>
#include <stdint.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum 
{
    ESP8266_STATE_READY = 0,
    ESP8266_STATE_ERROR,
    ESP8266_STATE_NUM,
    ESP8266_STATE_INV = 0xFF
}esp8266_state_t;


typedef void (*esp8266_cb_t)(esp8266_state_t, const char *); 

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void esp8266_init(void);
bool esp8266_netw_list(esp8266_cb_t cb);
bool esp8266_netw_con(const char * ssid, const char * pwd, esp8266_cb_t cb);
bool esp8266_netw_leave(esp8266_cb_t cb);
bool esp8266_netw_get_ssid(esp8266_cb_t cb);
bool esp8266_tcp_con(const char * ip, const char * port, esp8266_cb_t cb);
bool esp8266_tcp_leave(esp8266_cb_t cb);
bool esp8266_tcp_send(const void * data, uint16_t len);
bool esp8266_tcp_rec(void * data, int32_t * len);
bool esp8266_com_ready(void);
bool esp8266_busy(void);

/**********************
 *      MACROS
 **********************/

#endif /*USE_ESP8266 != 0*/

#endif /*ESP8266_H*/