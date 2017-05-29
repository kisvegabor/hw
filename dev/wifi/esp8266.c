/**
 * @file esp8266.c
 * 
 */



/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_ESP8266 != 0

#include "esp8266.h"
#include <string.h>
#include <stdio.h>


#include "misc/others/at.h"
#include "hw/dev/ui/log.h"
#include "hw/per/tick.h"
#include "hw/per/serial.h"
#include "hal/systick/systick.h"

/*********************
 *      DEFINES
 *********************/
#define ESP8266_TOUT_NETW_SSID      1000
#define ESP8266_TOUT_NETW_LEAVE     1000
#define ESP8266_TOUT_NETW_CON       20000
#define ESP8266_TOUT_NETW_LIST      10000
#define ESP8266_TOUT_TCP_CON        1000

/**********************
 *      TYPEDEFS
 **********************/
typedef enum
{
    ESP8266_TASK_NONE = 0,
    ESP8266_TASK_NETW_LIST,
    ESP8266_TASK_NETW_CON,
    ESP8266_TASK_NETW_LEAVE,
    ESP8266_TASK_NETW_SSID,
    ESP8266_TASK_NETW_IP,
    ESP8266_TASK_TCP_CON,
    ESP8266_TASK_TCP_LEAVE,
    ESP8266_TASK_NUM,
    ESP8266_TASK_INV = 0xFF
}esp8266_task_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void esp8266_handler(void * param);
static void netw_list_handler(void);
static void netw_con_handler(void);
static void netw_leave_handler(void);
static void netw_ssid_handler(void);
static void tcp_con_handler(void);
static void tcp_leave_handler(void);

static bool ssid_parser(char * buf);
static bool at_tx(char c);
static bool at_rx(char * c);


/**********************
 *  STATIC VARIABLES
 **********************/
static at_dsc_t at_dsc;
static esp8266_task_t act_task = ESP8266_TASK_NONE;
static esp8266_cb_t act_cb = NULL;
static uint8_t act_task_state = 0;
static uint32_t act_timestamp = 0;
static char last_ssid[64];
static char last_pwd[64];
static char last_tcp_ip[64];
static char last_tcp_port[64];
static bool com_ready = false;

LOG_FN("esp8266");

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void esp8266_init(void)
{
    
    static char com_buf[128];
    
    at_dsc.name = "esp8266";
    at_dsc.buf_len = 128;
    at_dsc.buf = com_buf;
    at_dsc.tx_char = at_tx;
    at_dsc.rx_char = at_rx;
    
    at_init_dsc(&at_dsc);
    
    at_send_str(&at_dsc, "ATE0\r\n");
    
    tick_wait_ms(200);
    serial_clear_rx_buf(HW_SERIAL5);
    
    at_state_t state;
    serial_clear_rx_buf(HW_SERIAL5);
    state = at_send_str(&at_dsc, "AT+CWMODE=3\r\n");
    if(state != AT_STATE_READY){
        SERR("AT+CWMODE failed during module init");
        return;
    }
    systick_delay_ms(100);
    state = at_get_line(&at_dsc);
    if(state != AT_STATE_READY){
        SERR("AT+CWMODE failed during module init");
        return;
    }
    
    if(strcmp("OK\r\n", at_dsc.buf) != 0) {
        SERR("AT+CWMODE failed during module init");
        return;
    }
    
    
    /*Transparent transmission mode*/
    state = at_send_str(&at_dsc, "AT+CIPMODE=1\r\n");
    if(state != AT_STATE_READY){
        SERR("AT+CIPMODE failed during module init");
        return;
    }
    systick_delay_ms(100);
    state = at_get_line(&at_dsc);
    if(state != AT_STATE_READY){
        SERR("AT+CIPMODE failed during module init");
        return;
    }
    
    if(strcmp("OK\r\n", at_dsc.buf) != 0) {
        SERR("AT+CIPMODE failed during module init");
        return;
    }
    
    
    ptask_create(esp8266_handler, 100, PTASK_PRIO_MID, NULL);
}


bool esp8266_netw_list(esp8266_cb_t cb)
{
    SMSG("Network list...");
    
    if(act_task != ESP8266_TASK_NONE) {
        SWARN("Can not list: busy");
        return false;
    }
    
    act_task_state = 0;
    act_timestamp = tick_get();
    act_cb = cb;
    serial_clear_rx_buf(HW_SERIAL5);
    act_task = ESP8266_TASK_NETW_LIST;
    
    return true;
    
}

bool esp8266_netw_con(const char * ssid, const char * pwd, esp8266_cb_t cb)
{
    
    SMSG("Network connect...");
    
    if(act_task != ESP8266_TASK_NONE) {
        SWARN("Can not connect: busy");
        return false;
    }
    
    strcpy(last_ssid, ssid);
    strcpy(last_pwd, pwd);
    
    act_task_state = 0;
    act_timestamp = tick_get();
    act_cb = cb;
    serial_clear_rx_buf(HW_SERIAL5);
    act_task = ESP8266_TASK_NETW_CON;
    
    return true;
}


bool esp8266_netw_leave(esp8266_cb_t cb)
{
    
    SMSG("Network leave...");
    
    if(act_task != ESP8266_TASK_NONE) {
        SWARN("Can not leave: busy");
        return false;
    }
    
    act_task_state = 0;
    act_timestamp = tick_get();
    act_cb = cb;
    serial_clear_rx_buf(HW_SERIAL5);
    act_task = ESP8266_TASK_NETW_LEAVE;
    
    return true;
}

bool esp8266_netw_get_ssid(esp8266_cb_t cb)
{
    SMSG("Get SSID...");
    
    if(act_task != ESP8266_TASK_NONE) {
        SWARN("Can not get SSID: busy");
        return false;
    }
    
    act_task_state = 0;
    act_timestamp = tick_get();
    act_cb = cb;
    serial_clear_rx_buf(HW_SERIAL5);
    act_task = ESP8266_TASK_NETW_SSID;
   
    return true; 
}

bool esp8266_tcp_con(const char * ip, const char * port, esp8266_cb_t cb)
{
    SMSG("Connecting to TCP server...");
    
    if(act_task != ESP8266_TASK_NONE) {
        SWARN("Can not connect: busy");
        return false;
    }
    
    strcpy(last_tcp_ip, ip);
    sprintf(last_tcp_port, "%s", port);
    act_task_state = 0;
    act_timestamp = tick_get();
    act_cb = cb;
    serial_clear_rx_buf(HW_SERIAL5);
    act_task = ESP8266_TASK_TCP_CON;
   
    return true;
}

bool esp8266_tcp_leave(esp8266_cb_t cb)
{
    SMSG("Leaving to TCP server...");
    
    if(act_task != ESP8266_TASK_NONE) {
        SWARN("Can not leave: busy");
        return false;
    }
    
    act_task_state = 0;
    act_timestamp = tick_get();
    act_cb = cb;
    serial_clear_rx_buf(HW_SERIAL5);
    act_task = ESP8266_TASK_TCP_LEAVE;
    
    return true;
}

bool esp8266_tcp_send(const void * data, uint16_t len)
{    
    if(act_task != ESP8266_TASK_NONE) {
        SWARN("Can not send to TCP server: busy");
        return false;
    }
    
    if(com_ready == false) { 
        SWARN("Can not send to TCP server: not connected");
        return false;
    }
    
    serial_send_force(HW_SERIAL5,data,len);
    
    return true;
}


bool esp8266_tcp_rec(void * data, int32_t * len)
{    
    if(act_task != ESP8266_TASK_NONE) {
        SWARN("Can not rec. from TCP server: busy");
        *len = 0;
        return false;
    }
    
    if(com_ready == false) { 
        SWARN("Can not read formTCP server: not connected");
        *len = 0;
        return false;
    }
    
    serial_rec(HW_SERIAL5, data, len);
    
    return true;
}

bool esp8266_com_ready(void)
{
    return com_ready;
}


bool esp8266_busy(void)
{
    return act_task == ESP8266_TASK_NONE ? false : true;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void esp8266_handler(void * param)
{
    if(act_task == ESP8266_TASK_NONE) {
        act_task_state = 0;
        
        if(com_ready != false) {
        }
        
        return;
    }
    else if(act_task == ESP8266_TASK_NETW_LIST) {
        netw_list_handler();
    } else if(act_task == ESP8266_TASK_NETW_LEAVE) {
        netw_leave_handler();
    } else if(act_task == ESP8266_TASK_NETW_CON) {
        netw_con_handler();
    } else if(act_task == ESP8266_TASK_NETW_SSID) {
        netw_ssid_handler();
    } else if(act_task == ESP8266_TASK_TCP_CON) {
        tcp_con_handler();
    } else if(act_task == ESP8266_TASK_TCP_LEAVE) {
        tcp_leave_handler();
    }     
}

static void netw_list_handler(void)
{
    at_state_t state;
    
    switch(act_task_state) {
        case 0:
            state = at_send_str(&at_dsc, "AT+CWLAP\r\n");
            if(state == AT_STATE_READY) {
                act_task_state ++;
            }
            else {
                SWARN("Network list: modem comm. error %s", at_get_state_text(&at_dsc));
                act_task = ESP8266_TASK_NONE;
                if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR, at_get_state_text(&at_dsc));
                return;
            }
            break;
        case 1:
            state = at_get_line(&at_dsc);
            if(state == AT_STATE_READY) {
                if(strcmp(at_dsc.buf, "OK\r\n") == 0) {
                    act_task = ESP8266_TASK_NONE;
                        SMSG("No more network to list");
                    if(act_cb != NULL) act_cb(ESP8266_STATE_READY, "");
                } else {
                    if(ssid_parser(at_dsc.buf) != false) {
                        SMSG("Network found: %s", at_dsc.buf);
                        if(act_cb != NULL) act_cb(ESP8266_STATE_READY, at_dsc.buf);
                    }
                }
            } 
            else if (state == AT_STATE_RX_INPROG) {
                if(tick_elaps(act_timestamp) > ESP8266_TOUT_NETW_LIST) {
                    SWARN("Timeouted");
                    act_cb(ESP8266_STATE_ERROR, "Timeout");
                    act_task = ESP8266_TASK_NONE;
                }
            } else {
                SWARN("Network list: modem comm. error %s", at_get_state_text(&at_dsc));
                act_cb(ESP8266_STATE_ERROR, at_get_state_text(&at_dsc));
                act_task = ESP8266_TASK_NONE;
            }
            break;
    }
}

static void netw_con_handler(void)
{
    at_state_t state;
    char buf[128];
    
     switch(act_task_state) {
        case 0:
            sprintf(buf,"AT+CWJAP_DEF=\"%s\",\"%s\"\r\n" , last_ssid, last_pwd);
            state = at_send_str(&at_dsc, buf);
            if(state != AT_STATE_READY) {
                SWARN("Network connect: modem comm. error %s", at_get_state_text(&at_dsc));
                act_task = ESP8266_TASK_NONE;
                act_cb(ESP8266_STATE_ERROR, at_get_state_text(&at_dsc));
            } else {
                act_task_state++;
            }
            break;
        case 1:
            state = at_get_line(&at_dsc);
            if(state == AT_STATE_READY) {
                if (strcmp(at_dsc.buf, "OK\r\n") == 0) {
                    act_task = ESP8266_TASK_NONE;
                    SMSG("Connected to network");
                    if(act_cb != NULL) act_cb(ESP8266_STATE_READY, "");
                } else if(strcmp(at_dsc.buf, "WIFI CONNECTED\r\n") == 0) {
                    SMSG("Connected to network");
                    act_task_state++;
                } else if(strcmp(at_dsc.buf, "WIFI DISCONNECT\r\n") == 0) {
                    SMSG("Current network leaved");
                } else if(strcmp(at_dsc.buf, "CLOSED\r\n") == 0) {
                    SMSG("Current network closed");
                } else {
                    SWARN("Wrong resp.: %s", at_dsc.buf);
                    if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR,  at_dsc.buf);
                act_task = ESP8266_TASK_NONE;
                }
            } 
            else if (state == AT_STATE_RX_INPROG) {
                if(tick_elaps(act_timestamp) > ESP8266_TOUT_NETW_CON) {
                    SWARN("Timeouted");
                    act_cb(ESP8266_STATE_ERROR, "Timeout");
                    act_task = ESP8266_TASK_NONE;
                }
            } else {
                SWARN("Network connect: modem comm. error %s", at_get_state_text(&at_dsc));
                act_cb(ESP8266_STATE_ERROR, at_get_state_text(&at_dsc));
                act_task = ESP8266_TASK_NONE;
            }
            break;
         case 2:
            state = at_get_line(&at_dsc);
            if(state == AT_STATE_READY) {
                if(strcmp(at_dsc.buf, "WIFI GOT IP\r\n") == 0 || 
                   strcmp(at_dsc.buf, "OK\r\n") ==  0) {
                    SMSG("IP received");
                    act_task = ESP8266_TASK_NONE;
                    if(act_cb != NULL) act_cb(ESP8266_STATE_READY, "");
                } else {
                    SWARN("Wrong resp.: %s", at_dsc.buf);
                    if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR,  at_dsc.buf);
                act_task = ESP8266_TASK_NONE;
                }
            } 
            else if (state == AT_STATE_RX_INPROG) {
                if(tick_elaps(act_timestamp) > ESP8266_TOUT_NETW_CON) {
                    SWARN("Timeouted");
                    act_cb(ESP8266_STATE_ERROR, "Timeout");
                    act_task = ESP8266_TASK_NONE;
                }
            } else {
                SWARN("Network connect: modem comm. error %s", at_get_state_text(&at_dsc));
                act_cb(ESP8266_STATE_ERROR, at_get_state_text(&at_dsc));
                act_task = ESP8266_TASK_NONE;
            }
           
             
     }
}


static void netw_leave_handler(void)
{
    at_state_t state;
    
     switch(act_task_state) {
        case 0:
            state = at_send_str(&at_dsc, "AT+CWQAP\r\n");
            if(state != AT_STATE_READY) {
                SWARN("Network leave: modem comm. error %s", at_get_state_text(&at_dsc));
                act_task = ESP8266_TASK_NONE;
                act_cb(ESP8266_STATE_ERROR, at_get_state_text(&at_dsc));
            } else {
                act_task_state++;
            }
            break;
        case 1:
            state = at_get_line(&at_dsc);
            if(state == AT_STATE_READY) {
                if(strcmp(at_dsc.buf, "OK\r\n") == 0) {
                    SMSG("Network leaved");
                    act_task = ESP8266_TASK_NONE;
                    if(act_cb != NULL) act_cb(ESP8266_STATE_READY, "");
                } else {
                    SMSG("Wrong resp.: %s", at_dsc.buf);
                    if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR,  at_dsc.buf);
                }
            } 
            else if (state == AT_STATE_RX_INPROG) {
                if(tick_elaps(act_timestamp) > ESP8266_TOUT_NETW_LEAVE) {
                    SWARN("Timeouted");
                    act_cb(ESP8266_STATE_ERROR, "Timeout");
                    act_task = ESP8266_TASK_NONE;
                }
            } else {
                SWARN("Network leave: modem comm. error %s", at_get_state_text(&at_dsc));
                act_cb(ESP8266_STATE_ERROR, at_get_state_text(&at_dsc));
                act_task = ESP8266_TASK_NONE;
            }
            break;
     }
}

static void netw_ssid_handler(void)
{
    at_state_t state;
    
     switch(act_task_state) {
        case 0:
            state = at_send_str(&at_dsc, "AT+CWJAP?\r\n");
            if(state != AT_STATE_READY) {
                SWARN("Network get SSID: modem comm. error %s", at_get_state_text(&at_dsc));
                act_task = ESP8266_TASK_NONE;
                act_cb(ESP8266_STATE_ERROR, at_get_state_text(&at_dsc));
            } else {
                act_task_state++;
            }
            break;
         case 1:
             state = at_get_line(&at_dsc);
             if(state == AT_STATE_READY) {
                 if(strcmp("No AP\r\n", at_dsc.buf) == 0) {
                     act_task_state = 2;
                 } else {
                    if(ssid_parser(at_dsc.buf) != false) {
                        SMSG("Connected to: %s", at_dsc.buf);
                        if(act_cb != NULL) act_cb(ESP8266_STATE_READY, at_dsc.buf);
                        act_task = ESP8266_TASK_NONE;
                    }
                 }
             } else if(state == AT_STATE_RX_INPROG) { 
                 if(tick_elaps(act_timestamp) > ESP8266_TOUT_NETW_SSID) {
                    SWARN("Timeouted");
                    act_cb(ESP8266_STATE_ERROR, "Timeout");
                    act_task = ESP8266_TASK_NONE;
                }
             } else {
                SWARN("Network get SSID: modem comm. error %s", at_get_state_text(&at_dsc));
                act_cb(ESP8266_STATE_ERROR, at_get_state_text(&at_dsc));
                act_task = ESP8266_TASK_NONE;
            }
            break;
         case 2: 
             state = at_get_line(&at_dsc);
             if(state == AT_STATE_READY) {
                 if(strcmp("OK\r\n", at_dsc.buf) == 0) {
                     SMSG("Connected to: none");
                     if(act_cb != NULL) act_cb(ESP8266_STATE_READY, "");
                     act_task = ESP8266_TASK_NONE;
                 } else if(strcmp(at_dsc.buf, "busy p...\r\n") == 0) {
                     SMSG("Network get SSID: busy p...");
                 } else {
                     SWARN("Wrong resp.: %s", at_dsc.buf);
                     act_cb(ESP8266_STATE_ERROR, at_get_state_text(&at_dsc));
                     act_task = ESP8266_TASK_NONE;
                 }
             } else if(state == AT_STATE_RX_INPROG) { 
                 if(tick_elaps(act_timestamp) > ESP8266_TOUT_NETW_SSID) {
                    SWARN("Timeouted");
                    act_cb(ESP8266_STATE_ERROR, "Timeout");
                    act_task = ESP8266_TASK_NONE;
                }
             } else {
                SWARN("Network get SSID: modem comm. error %s", at_get_state_text(&at_dsc));
                act_cb(ESP8266_STATE_ERROR, at_get_state_text(&at_dsc));
                act_task = ESP8266_TASK_NONE;
            }
             
             break;
     }
}




static void tcp_con_handler(void)
{ 
    at_state_t state;
    char buf[256];
    
     switch(act_task_state) {
        case 0:
            sprintf(buf, "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n", last_tcp_ip, last_tcp_port);
            state = at_send_str(&at_dsc, buf);
            if(state != AT_STATE_READY) {
                SWARN("TCP connect: modem comm. error %s", at_get_state_text(&at_dsc));
                act_task = ESP8266_TASK_NONE;
                act_cb(ESP8266_STATE_ERROR, at_get_state_text(&at_dsc));
            } else {
                act_task_state++;
            }
            break;
        case 1:
            state = at_get_line(&at_dsc);
            if(state == AT_STATE_READY) {
                if(strcmp(at_dsc.buf, "OK\r\n") == 0 ||
                   strcmp(at_dsc.buf, "ALREADY CONNECTED\r\n") == 0) {
                    SMSG("Connected to TCP server");
                    act_task_state++;
                } else if(strcmp(at_dsc.buf, "busy p...\r\n") == 0) {
                    SMSG("Connecting to TCP server...");
                } else if(strcmp(at_dsc.buf, "CONNECT\r\n") == 0) {
                    SMSG("Connecting to TCP server...");
                } else {
                    SWARN("Wrong resp.: %s", at_dsc.buf);
                    act_task = ESP8266_TASK_NONE;
                    if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR,  at_dsc.buf);
                }
            } 
            else if (state == AT_STATE_RX_INPROG) {
                if(tick_elaps(act_timestamp) > ESP8266_TOUT_TCP_CON) {
                    SWARN("Timeouted");
                    act_cb(ESP8266_STATE_ERROR, "Timeout");
                    act_task = ESP8266_TASK_NONE;
                }
            } else {
                SWARN("TCP connect: modem comm. error %s", at_get_state_text(&at_dsc));
                act_cb(ESP8266_STATE_ERROR, at_get_state_text(&at_dsc));
                act_task = ESP8266_TASK_NONE;
            }
            break;
         case 2:
            state = at_send_str(&at_dsc,"AT+CIPSEND\r\n");
            if(state != AT_STATE_READY) {
                SWARN("TCP connect: modem comm. error %s", at_get_state_text(&at_dsc));
                act_task = ESP8266_TASK_NONE;
                act_cb(ESP8266_STATE_ERROR, at_get_state_text(&at_dsc));
            } else {
                act_task_state++;
            }
         case 3:
            state = at_get_line(&at_dsc);
            if(state == AT_STATE_READY) {
                if(strcmp(at_dsc.buf, ">\r\n") == 0 ||
                   strcmp(at_dsc.buf, "OK\r\n") == 0) {
                    act_task = ESP8266_TASK_NONE;
                    SMSG("TCP pass through mode. Ready.");
                    act_task = ESP8266_TASK_NONE;
                    com_ready = true;
                    if(act_cb != NULL) act_cb(ESP8266_STATE_READY,  "");
                } else {
                    SWARN("Wrong resp.: %s", at_dsc.buf);
                    act_task = ESP8266_TASK_NONE;
                    if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR,  at_dsc.buf);
                }
            } 
            else if (state == AT_STATE_RX_INPROG) {
                if(tick_elaps(act_timestamp) > ESP8266_TOUT_TCP_CON) {
                    SWARN("Timeouted");
                    act_cb(ESP8266_STATE_ERROR, "Timeout");
                    act_task = ESP8266_TASK_NONE;
                }
            } else {
                SWARN("TCP connect: modem comm. error %s", at_get_state_text(&at_dsc));
                act_cb(ESP8266_STATE_ERROR, at_get_state_text(&at_dsc));
                act_task = ESP8266_TASK_NONE;
            }
            break;
     }
}
    

static void tcp_leave_handler(void)
{ 
    at_state_t state;
    
     switch(act_task_state) {
        case 0:
            state = at_send_data(&at_dsc, "+++", 3);
            if(state != AT_STATE_READY) {
                SWARN("TCP leave: modem comm. error %s", at_get_state_text(&at_dsc));
                act_task = ESP8266_TASK_NONE;
                act_cb(ESP8266_STATE_ERROR, at_get_state_text(&at_dsc));
            } else {
                act_task_state++;
            }
            break;
         case 1:
             if(systick_elaps(act_timestamp) > 1000) {
                SWARN("TCP server leaved");
                act_task = ESP8266_TASK_NONE;
                act_cb(ESP8266_STATE_READY, at_get_state_text(&at_dsc));
             }
             break;
     }
}
    
static bool ssid_parser(char * buf)
{
    uint16_t i = 0, j = 0;
    while(buf[i] != '\"' && buf[i] != '\0') i++;  /*Find the first  " */
    
    if(buf[i] == '\0') return false;    /*Invalid answer*/
    
    i++;    /*Move after the " sign*/
    
    while(buf[i] != '\"' && buf[i] != '\0') {  /*Copy the ssid to the beginning */
        buf[j] = buf[i];
        j++;
        i++;
    }
    buf[j] = '\0';
    
    return true;
}

static bool at_tx(char c)
{
    int32_t l = 1;
    hw_res_t res;
    res = serial_send(HW_SERIAL5, &c, &l);
    
    if(res != HW_RES_OK) return false;
    if(l != 1) return false;
    
    return true;
}


static bool at_rx(char * c)
{
    int32_t l = 1;
    hw_res_t res;
    res = serial_rec(HW_SERIAL5, c, &l);
    if(res != HW_RES_OK) return false;
    if(l != 1) return false;
    
    return true;
}
#endif /*USE_ESP8266 != 0*/