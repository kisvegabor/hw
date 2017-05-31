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

#include "hw/dev/ui/log.h"
#include "hw/per/tick.h"
#include "hw/per/serial.h"
#include "misc/os/ptask.h"
#include "hal/systick/systick.h"

/*********************
 *      DEFINES
 *********************/
#define ESP8266_TOUT_NETW_SSID      1000
#define ESP8266_TOUT_NETW_LEAVE     1000
#define ESP8266_TOUT_NETW_CON       20000
#define ESP8266_TOUT_NETW_LIST      10000
#define ESP8266_TOUT_TCP_CON        5000

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

static bool read_line(void);


/**********************
 *  STATIC VARIABLES
 **********************/
static esp8266_task_t act_task = ESP8266_TASK_NONE;
static esp8266_cb_t act_cb = NULL;
static uint8_t act_task_state = 0;
static uint32_t act_timestamp = 0;
static char last_ssid[64];
static char last_pwd[64];
static char last_tcp_ip[64];
static char last_tcp_port[64];
static bool com_ready = false;

static char line_buf[256];
static uint16_t line_i;

LOG_FN("esp8266");

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void esp8266_init(void)
{
    
    serial_send_force(HW_SERIAL5, "ATE0\r\n", SERIAL_SEND_STRING);
    
    tick_wait_ms(200);
    
    hw_res_t send_res;
    bool read_res;
    serial_clear_rx_buf(HW_SERIAL5);
    line_i = 0;
    send_res = serial_send_force(HW_SERIAL5, "AT+CWMODE=3\r\n", SERIAL_SEND_STRING);
    if(send_res != HW_RES_OK){
        SERR("AT+CWMODE failed during module init (1)");
        return;
    }
    systick_delay_ms(100);
    read_res = read_line();
    if(read_res == false){
        SERR("AT+CWMODE failed during module init (2)");
        return;
    }
    
    if(strcmp("OK\r\n", line_buf) != 0) {
        SERR("AT+CWMODE failed during module init (3)");
        return;
    }
    
    
    /*Transparent transmission mode*/
    send_res = serial_send_force(HW_SERIAL5, "AT+CIPMODE=1\r\n", SERIAL_SEND_STRING);
    if(send_res != HW_RES_OK){
        SERR("AT+CIPMODE failed during module init (1)");
        return;
    }
    systick_delay_ms(100);
    read_res = read_line();
    if(read_res == false){
        SERR("AT+CIPMODE failed during module init (2)");
        return;
    }
    
    if(strcmp("OK\r\n", line_buf) != 0) {
        SERR("AT+CIPMODE failed during module init (3)");
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
    line_i = 0;
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
    line_i = 0;
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
    line_i = 0;
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
    line_i = 0;
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
    line_i = 0;
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
    line_i = 0;
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
    bool read_res;
    hw_res_t send_res;
    
    switch(act_task_state) {
        case 0:
            send_res = serial_send_force(HW_SERIAL5, "AT+CWLAP\r\n", SERIAL_SEND_STRING);
            if(send_res == HW_RES_OK) {
                act_task_state ++;
            }
            else {
                SWARN("Network list: modem comm. error");
                act_task = ESP8266_TASK_NONE;
                if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR, "Communication error");
                return;
            }
            break;
        case 1:
            read_res = read_line();
            if(read_res != false) {
                if(strcmp(line_buf, "OK\r\n") == 0) {
                    act_task = ESP8266_TASK_NONE;
                    SMSG("No more network to list");
                    if(act_cb != NULL) act_cb(ESP8266_STATE_READY, "");
                } else {
                    if(ssid_parser(line_buf) != false) {
                        SMSG("Network found: %s", line_buf);
                        if(act_cb != NULL) act_cb(ESP8266_STATE_READY, line_buf);
                    }
                }
            } 
            else {
                if(tick_elaps(act_timestamp) > ESP8266_TOUT_NETW_LIST) {
                    SWARN("Timeouted");
                    line_i = 0;
                    act_cb(ESP8266_STATE_ERROR, "Timeout");
                    act_task = ESP8266_TASK_NONE;
                }
            } 
            break;
    }
}

static void netw_con_handler(void)
{
    bool read_res;
    hw_res_t send_res;
    char buf[128];
    
     switch(act_task_state) {
        case 0:
            sprintf(buf,"AT+CWJAP_DEF=\"%s\",\"%s\"\r\n" , last_ssid, last_pwd);
            send_res = serial_send_force(HW_SERIAL5, buf, SERIAL_SEND_STRING);
            if(send_res != HW_RES_OK) {
                SWARN("Network connect: modem comm. error");
                act_task = ESP8266_TASK_NONE;
                if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR, "Communication error");
            } else {
                act_task_state++;
            }
            break;
        case 1:
            read_res = read_line();
            if(read_res != false) {
                if (strcmp(line_buf, "OK\r\n") == 0) {
                    act_task = ESP8266_TASK_NONE;
                    SMSG("Connected to network");
                    if(act_cb != NULL) act_cb(ESP8266_STATE_READY, last_ssid);
                } else if(strcmp(line_buf, "WIFI CONNECTED\r\n") == 0) {
                    SMSG("Connected to network");
                    act_task_state++;
                } else if(strcmp(line_buf, "WIFI DISCONNECT\r\n") == 0) {
                    SMSG("Current network leaved");
                } else if(strcmp(line_buf, "CLOSED\r\n") == 0) {
                    SMSG("Current network closed");
                } else {
                    SWARN("Wrong resp.: %s", line_buf);
                    if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR,  line_buf);
                act_task = ESP8266_TASK_NONE;
                }
            } 
            else {
                if(tick_elaps(act_timestamp) > ESP8266_TOUT_NETW_CON) {
                    SWARN("Timeouted");
                    act_cb(ESP8266_STATE_ERROR, "Timeout");
                    act_task = ESP8266_TASK_NONE;
                }
            } 
            break;
         case 2:
            read_res = read_line();
            if(read_res != false) {
                if(strcmp(line_buf, "WIFI GOT IP\r\n") == 0 || 
                   strcmp(line_buf, "OK\r\n") ==  0) {
                    SMSG("IP received");
                    act_task = ESP8266_TASK_NONE;
                    if(act_cb != NULL) act_cb(ESP8266_STATE_READY, last_ssid);
                } else {
                    SWARN("Wrong resp.: %s", line_buf);
                    if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR,  line_buf);
                act_task = ESP8266_TASK_NONE;
                }
            } 
            else{
                if(tick_elaps(act_timestamp) > ESP8266_TOUT_NETW_CON) {
                    SWARN("Timeouted");
                    act_cb(ESP8266_STATE_ERROR, "Timeout");
                    act_task = ESP8266_TASK_NONE;
                }
            } 
     }
}


static void netw_leave_handler(void)
{
    bool read_res;
    hw_res_t send_res;
    
     switch(act_task_state) {
        case 0:
            send_res = serial_send_force(HW_SERIAL5, "AT+CWQAP\r\n", SERIAL_SEND_STRING);
            if(send_res != HW_RES_OK) {
                SWARN("Network leave: modem comm. error");
                act_task = ESP8266_TASK_NONE;
                if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR, "Communication error");
            } else {
                act_task_state++;
            }
            break;
        case 1:
            read_res = read_line();
            if(read_res != false) {
                if(strcmp(line_buf, "OK\r\n") == 0) {
                    SMSG("Network leaved");
                    act_task = ESP8266_TASK_NONE;
                    if(act_cb != NULL) act_cb(ESP8266_STATE_READY, "");
                } else {
                    SMSG("Wrong resp.: %s", line_buf);
                    if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR, line_buf);
                }
            } 
            else {
                if(tick_elaps(act_timestamp) > ESP8266_TOUT_NETW_LEAVE) {
                    SWARN("Timeouted");
                    act_cb(ESP8266_STATE_ERROR, "Timeout");
                    act_task = ESP8266_TASK_NONE;
                }
            } 
            break;
     }
}

static void netw_ssid_handler(void)
{
    bool read_res;
    hw_res_t send_res;
    
     switch(act_task_state) {
        case 0:
            send_res = serial_send_force(HW_SERIAL5, "AT+CWJAP?\r\n", SERIAL_SEND_STRING);
            if(send_res != HW_RES_OK) {
                SWARN("Network get SSID: modem comm. error");
                act_task = ESP8266_TASK_NONE;
                if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR, "Communication error");
            } else {
                act_task_state++;
            }
            break;
         case 1:
             read_res = read_line();
             if(read_res != false) {
                 if(strcmp("No AP\r\n", line_buf) == 0) {
                     act_task_state = 2;
                 } else {
                    if(ssid_parser(line_buf) != false) {
                        SMSG("Connected to: %s", line_buf);
                        if(act_cb != NULL) act_cb(ESP8266_STATE_READY, line_buf);
                        act_task = ESP8266_TASK_NONE;
                    }
                 }
             } else { 
                 if(tick_elaps(act_timestamp) > ESP8266_TOUT_NETW_SSID) {
                    SWARN("Timeouted");
                    act_cb(ESP8266_STATE_ERROR, "Timeout");
                    act_task = ESP8266_TASK_NONE;
                }
             } 
            break;
         case 2: 
             read_res = read_line();
             if(read_res != false) {
                 if(strcmp("OK\r\n", line_buf) == 0) {
                     SMSG("Connected to: none");
                     if(act_cb != NULL) act_cb(ESP8266_STATE_READY, "");
                     act_task = ESP8266_TASK_NONE;
                 } else if(strcmp(line_buf, "busy p...\r\n") == 0) {
                     SMSG("Network get SSID: busy p...");
                 } else {
                     SWARN("Wrong resp.: %s", line_buf);
                     act_cb(ESP8266_STATE_ERROR, line_buf);
                     act_task = ESP8266_TASK_NONE;
                 }
             } else { 
                 if(tick_elaps(act_timestamp) > ESP8266_TOUT_NETW_SSID) {
                    SWARN("Timeouted");
                    act_cb(ESP8266_STATE_ERROR, "Timeout");
                    act_task = ESP8266_TASK_NONE;
                }
             } 
             break;
     }
}

static void tcp_con_handler(void)
{ 
    bool read_res;
    hw_res_t send_res;
    char buf[256];
    
     switch(act_task_state) {
        case 0:
            sprintf(buf, "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n", last_tcp_ip, last_tcp_port);
            send_res = serial_send_force(HW_SERIAL5, buf, SERIAL_SEND_STRING);
            if(send_res != HW_RES_OK) {
                SWARN("TCP connect: modem comm. error");
                act_task = ESP8266_TASK_NONE;
                if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR, "Communication error");
            } else {
                act_task_state++;
            }
            break;
        case 1:
            read_res = read_line();
            if(read_res != false) {
                if(strcmp(line_buf, "OK\r\n") == 0 ||
                   strcmp(line_buf, "ALREADY CONNECTED\r\n") == 0) {
                    SMSG("Connected to TCP server");
                    act_task_state++;
                } else if(strcmp(line_buf, "busy p...\r\n") == 0) {
                    SMSG("Connecting to TCP server...");
                } else if(strcmp(line_buf, "CONNECT\r\n") == 0) {
                    SMSG("Connecting to TCP server...");
                } else {
                    SWARN("Wrong resp.: %s", line_buf);
                    act_task = ESP8266_TASK_NONE;
                    if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR, line_buf);
                }
            } 
            else {
                if(tick_elaps(act_timestamp) > ESP8266_TOUT_TCP_CON) {
                    SWARN("Timeouted");
                    act_cb(ESP8266_STATE_ERROR, "Timeout");
                    act_task = ESP8266_TASK_NONE;
                }
            } 
            break;
         case 2:
            send_res = serial_send_force(HW_SERIAL5, "AT+CIPSEND\r\n", SERIAL_SEND_STRING);
            if(send_res != HW_RES_OK) {
                SWARN("TCP connect: modem comm. error");
                act_task = ESP8266_TASK_NONE;
                if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR, "Communication error");
            } else {
                act_task_state++;
            }
         case 3:
            read_res = read_line();
            if(read_res != false) {
                if(strcmp(line_buf, ">\r\n") == 0 ||
                   strcmp(line_buf, "OK\r\n") == 0) {
                    act_task = ESP8266_TASK_NONE;
                    SMSG("TCP pass through mode. Ready.");
                    act_task = ESP8266_TASK_NONE;
                    com_ready = true;
                    if(act_cb != NULL) act_cb(ESP8266_STATE_READY,  "");
                } else {
                    SWARN("Wrong resp.: %s", line_buf);
                    act_task = ESP8266_TASK_NONE;
                    if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR, line_buf);
                }
            } 
            else {
                if(tick_elaps(act_timestamp) > ESP8266_TOUT_TCP_CON) {
                    SWARN("Timeouted");
                    act_cb(ESP8266_STATE_ERROR, "Timeout");
                    act_task = ESP8266_TASK_NONE;
                }
            } 
            break;
     }
}
    

static void tcp_leave_handler(void)
{ 
    hw_res_t send_res;
    
     switch(act_task_state) {
        case 0:
            send_res = serial_send_force(HW_SERIAL5, "+++", 3);
            if(send_res != HW_RES_OK) {
                SWARN("TCP leave: modem comm. error");
                act_task = ESP8266_TASK_NONE;
                if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR, "Communication error");
            } else {
                act_task_state++;
            }
            break;
         case 1:
             if(systick_elaps(act_timestamp) > 1000) {
                SWARN("TCP server leaved");
                act_task = ESP8266_TASK_NONE;
                act_cb(ESP8266_STATE_READY, "");
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

static bool read_line(void)
{   
    hw_res_t res;
    int32_t length;
    while(line_i < sizeof(line_buf)) {
        length = 1;
        res = serial_rec(HW_SERIAL5, &line_buf[line_i], &length);
        if(res == HW_RES_OK && length == 1) {
            /*Ignore leading '\r' and '\n'*/
            if(line_i != 0 || (line_buf[0] != '\n' && line_buf[0] != '\r')) 
            {
//                char buf[2];
//                buf[0] = line_buf[line_i];
//                buf[1] = '\0';
//                SMSG("%s", buf);
                
                line_i ++;

                /*Check line ending*/
                if(line_i >= 2) {
                    if(line_buf[line_i - 1] == '\n' &&
                       line_buf[line_i - 2] == '\r') 
                    {
                        line_buf[line_i] = '\0';   /*Close the sting*/
                        line_i = 0;
                        SMSG("Line received: %s", line_buf);
                        return true;
                    }
                }
            }
        } else {
            return false;
        }
    }
    
    if(line_i == sizeof(line_buf)) {
        SWARN("Overflow");
        line_i = 0;
    }
    
    return false;
}
#endif /*USE_ESP8266 != 0*/