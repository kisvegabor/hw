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

/*********************
 *      DEFINES
 *********************/
#define ESP8266_TOUT_NETW_SSID      1000
#define ESP8266_TOUT_NETW_LEAVE     1000
#define ESP8266_TOUT_NETW_CON       20000
#define ESP8266_TOUT_NETW_LIST      10000
#define ESP8266_TOUT_TCP_CON        5000
#define ESP8266_TOUT_TCP_LEAVE      5000
#define ESP8266_TOUT_TCP_TRANSF     10000

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
    ESP8266_TASK_TCP_TRANSF,
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
static void tcp_transf_handler(void);

static bool ssid_parser(char * buf);

static bool read_line(void);


/**********************
 *  STATIC VARIABLES
 **********************/
static esp8266_task_t act_task = ESP8266_TASK_NONE;
static esp8266_cb_t act_cb = NULL;
static uint8_t act_task_state = 0;
static uint32_t act_timestamp = 0;
static char last_param1[64];
static char last_param2[64];
static bool com_ready = false;

static char transf_buf[ESP8266_BUF_SIZE];
static int transf_size;
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
    
    serial_send_force(ESP8266_DRV, "ATE0\r\n", SERIAL_SEND_STRING);
    
    tick_wait_ms(200);
    
    hw_res_t send_res;
    bool read_res;
    serial_clear_rx_buf(ESP8266_DRV);
    line_i = 0;
    send_res = serial_send_force(ESP8266_DRV, "AT+CWMODE=3\r\n", SERIAL_SEND_STRING);
    if(send_res != HW_RES_OK){
        SERR("AT+CWMODE failed during module init (1)");
        return;
    }
    tick_wait_ms(100);
    read_res = read_line();
    if(read_res == false){
        SERR("AT+CWMODE failed during module init (2)");
        return;
    }
    
    if(strcmp("OK\r\n", line_buf) != 0) {
        SERR("AT+CWMODE failed during module init (3)");
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
    serial_clear_rx_buf(ESP8266_DRV);
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
    
    strcpy(last_param1, ssid);
    strcpy(last_param2, pwd);
    
    act_task_state = 0;
    act_timestamp = tick_get();
    act_cb = cb;
    serial_clear_rx_buf(ESP8266_DRV);
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
    serial_clear_rx_buf(ESP8266_DRV);
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
    serial_clear_rx_buf(ESP8266_DRV);
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
    
    strcpy(last_param1, ip);
    strcpy(last_param2, port);
    act_task_state = 0;
    act_timestamp = tick_get();
    act_cb = cb;
    serial_clear_rx_buf(ESP8266_DRV);
    line_i = 0;
    act_task = ESP8266_TASK_TCP_CON;
   
    return true;
}

bool esp8266_tcp_leave(esp8266_cb_t cb)
{
    SMSG("Leaving TCP server...");
    
    if(act_task != ESP8266_TASK_NONE) {
        SWARN("Can not leave: busy");
        return false;
    }
    
    act_task_state = 0;
    act_timestamp = tick_get();
    act_cb = cb;
    serial_clear_rx_buf(ESP8266_DRV);
    line_i = 0;
    act_task = ESP8266_TASK_TCP_LEAVE;
    
    return true;
}

bool esp8266_tcp_transf(const void * data, uint16_t len, esp8266_cb_t cb)
{    
    if(act_task != ESP8266_TASK_NONE) {
        SWARN("Can not send to TCP server: busy");
        return false;
    }
    
    if(com_ready == false) {
        SWARN("Not ready for comm. (Not connected)");
        return false;
    }
    
    memcpy(transf_buf, data, len);
    transf_size = len;
    act_task_state = 0;
    act_timestamp = tick_get();
    act_cb = cb;
    serial_clear_rx_buf(ESP8266_DRV);
    line_i = 0;
    act_task = ESP8266_TASK_TCP_TRANSF;
    
    return true;
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
    } else if(act_task == ESP8266_TASK_TCP_TRANSF) {
        tcp_transf_handler();
    }        
}

static void netw_list_handler(void)
{
    bool read_res;
    
    switch(act_task_state) {
        case 0:
            serial_send_force(ESP8266_DRV, "AT+CWLAP\r\n", SERIAL_SEND_STRING);
            act_task_state ++;
            break;
        case 1:
            read_res = read_line();
            if(read_res != false) {
                if(strcmp(line_buf, "OK\r\n") == 0) {
                    SMSG("No more network to list");
                    act_task = ESP8266_TASK_NONE;
                    if(act_cb != NULL) act_cb(ESP8266_STATE_READY, "");
                } else {
                    if(ssid_parser(line_buf) != false) {
                        SMSG("Network found: %s", line_buf);
                        if(act_cb != NULL) act_cb(ESP8266_STATE_READY, line_buf);
                    }
                }
            } 
            break;
    }
    if(tick_elaps(act_timestamp) > ESP8266_TOUT_NETW_LIST) {
        SWARN("Timeouted");
        line_i = 0;
        act_cb(ESP8266_STATE_ERROR, "Timeout");
        act_task = ESP8266_TASK_NONE;
    }
}

static void netw_con_handler(void)
{
    bool read_res;
    char buf[128];
    
     switch(act_task_state) {
        case 0:
            sprintf(buf,"AT+CWJAP=\"%s\",\"%s\"\r\n" , last_param1, last_param2);
            serial_send_force(ESP8266_DRV, buf, SERIAL_SEND_STRING);
            act_task_state++;
            break;
        case 1:
            read_res = read_line();
            if(read_res != false) {
                if (strcmp(line_buf, "OK\r\n") == 0) {
                    act_task = ESP8266_TASK_NONE;
                    SMSG("Connected to network");
                    if(act_cb != NULL) act_cb(ESP8266_STATE_READY, last_param1);
                } else if(strcmp(line_buf, "WIFI CONNECTED\r\n") == 0) {
                    SMSG("Connected to network");
                    act_task_state++;
                } else if(strcmp(line_buf, "WIFI DISCONNECT\r\n") == 0) {
                    SMSG("Current network leaved");
                } else if(strcmp(line_buf, "CLOSED\r\n") == 0) {
                    SMSG("Current network closed");
                } else {
                    SWARN("Wrong resp.: %s", line_buf);
                    act_task = ESP8266_TASK_NONE;
                    if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR,  line_buf);
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
                    if(act_cb != NULL) act_cb(ESP8266_STATE_READY, last_param1);
                } else {
                    SWARN("Wrong resp.: %s", line_buf);
                    act_task = ESP8266_TASK_NONE;
                    if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR,  line_buf);
                }
            } 
            break;
     }
     
     if(tick_elaps(act_timestamp) > ESP8266_TOUT_NETW_CON) {
        SWARN("Timeouted");
        act_task = ESP8266_TASK_NONE;
        act_cb(ESP8266_STATE_ERROR, "Timeout");
    }
}


static void netw_leave_handler(void)
{
    bool read_res;
    
     switch(act_task_state) {
        case 0:
            serial_send_force(ESP8266_DRV, "AT+CWQAP\r\n", SERIAL_SEND_STRING);
            act_task_state++;
            break;
        case 1:
            read_res = read_line();
            if(read_res != false) {
                if(strcmp(line_buf, "OK\r\n") == 0) {
                    SMSG("Network leaved");
                    com_ready = false;
                    act_task = ESP8266_TASK_NONE;
                    if(act_cb != NULL) act_cb(ESP8266_STATE_READY, "");
                } else {
                    SMSG("Wrong resp.: %s", line_buf);
                    if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR, line_buf);
                }
            } 
            break;
     }
     
     if(tick_elaps(act_timestamp) > ESP8266_TOUT_NETW_LEAVE) {
        SWARN("Timeouted");
        act_cb(ESP8266_STATE_ERROR, "Timeout");
        act_task = ESP8266_TASK_NONE;
    }
}

static void netw_ssid_handler(void)
{
    bool read_res;
    
     switch(act_task_state) {
        case 0:
            serial_send_force(ESP8266_DRV, "AT+CWJAP?\r\n", SERIAL_SEND_STRING);
            act_task_state++;
            break;
         case 1:
             read_res = read_line();
             if(read_res != false) {
                 if(strcmp("No AP\r\n", line_buf) == 0) {
                     act_task_state ++;
                 } else {
                    if(ssid_parser(line_buf) != false) {
                        SMSG("Connected to: %s", line_buf);
                        if(act_cb != NULL) act_cb(ESP8266_STATE_READY, line_buf);
                        act_task = ESP8266_TASK_NONE;
                    }
                 }
             }
            break;
         case 2: 
             read_res = read_line();
             if(read_res != false) {
                 if(strcmp("OK\r\n", line_buf) == 0) {
                     SMSG("Connected to: none");
                    act_task = ESP8266_TASK_NONE;
                     if(act_cb != NULL) act_cb(ESP8266_STATE_READY, "");
                 } else if(strcmp(line_buf, "busy p...\r\n") == 0) {
                     SMSG("Network get SSID: busy p...");
                 } else {
                    SWARN("Wrong resp.: %s", line_buf);
                    act_task = ESP8266_TASK_NONE;
                    act_cb(ESP8266_STATE_ERROR, line_buf);
                 }
             }
             break;
     }
     
    if(tick_elaps(act_timestamp) > ESP8266_TOUT_NETW_SSID) {
       SWARN("Timeouted");
       act_task = ESP8266_TASK_NONE;
       act_cb(ESP8266_STATE_ERROR, "Timeout");
   }
}

static void tcp_con_handler(void)
{ 
    bool read_res;
    char buf[256];
    
     switch(act_task_state) {
        case 0:
            sprintf(buf, "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n", last_param1, last_param2);
            serial_send_force(ESP8266_DRV, buf, SERIAL_SEND_STRING);
            act_task_state++;
            break;
        case 1:
            read_res = read_line();
            if(read_res != false) {
                if(strcmp(line_buf, "OK\r\n") == 0 ||
                   strcmp(line_buf, "ALREADY CONNECTED\r\n") == 0) {
                    SMSG("Connected to TCP server");
                    com_ready = true;
                    act_task = ESP8266_TASK_NONE;
                    sprintf(buf, "%s:%s", last_param1, last_param2);
                    if(act_cb != NULL) act_cb(ESP8266_STATE_READY, buf);
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
            break;
        
     }
     
    if(tick_elaps(act_timestamp) > ESP8266_TOUT_TCP_CON) {
        SWARN("Timeouted");
        act_task = ESP8266_TASK_NONE;
        act_cb(ESP8266_STATE_ERROR, "Timeout");
    }
}
    

static void tcp_leave_handler(void)
{ 
    bool read_res;
    
     switch(act_task_state) {
        case 0:
            serial_send_force(ESP8266_DRV, "AT+CIPCLOSE\r\n", SERIAL_SEND_STRING);
            act_task_state++;
            break;
         case 1:
            read_res = read_line();
            if(read_res != false) {
                if(strcmp(line_buf, "CLOSED\r\n") == 0) {
                    com_ready = false;
                    act_task_state++;
                } else {
                    SWARN("TCP Connection close error 1");
                    act_task = ESP8266_TASK_NONE;
                    if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR, line_buf); 
                }
            }
            break;
         case 2:
            read_res = read_line();
            if(read_res != false) {
                if(strcmp(line_buf, "OK\r\n") == 0) {
                    SMSG("TCP Connection closed");
                    act_task = ESP8266_TASK_NONE;
                    if(act_cb != NULL) act_cb(ESP8266_STATE_READY, "");
                } else {
                    SWARN("TCP Connection close error 2");
                    act_task = ESP8266_TASK_NONE;
                    if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR, line_buf); 
                }
            }
            break;
     } 
     if(tick_elaps(act_timestamp) > ESP8266_TOUT_TCP_LEAVE) {
        SWARN("Timeout");
        act_task = ESP8266_TASK_NONE;
        act_cb(ESP8266_STATE_ERROR, "Timeout");
    }
}

static void tcp_transf_handler(void)
{
    hw_res_t read_res;
    char buf[256];
    int32_t len;
    static uint8_t recp = 0;
    
     switch(act_task_state) {
        case 0: /*Send data length*/
            sprintf(buf, "AT+CIPSEND=%d\r\n", transf_size);
            serial_send_force(ESP8266_DRV, buf, SERIAL_SEND_STRING);
            act_task_state++;
            break;
            
        case 1: /*Wait for OK*/
            if(read_line() != false) {
                if(strcmp(line_buf, "OK\r\n") == 0) {
                    SMSG("Data send OK received");
                    act_task_state ++;
                } else {
                    SWARN("Wrong resp.: %s", line_buf);
                    act_task = ESP8266_TASK_NONE;
                    if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR, line_buf);
                }
            } 
            break;
            
            
         case 2: /*Wait for ">" and send data*/
            len = 1;
            read_res = serial_rec(ESP8266_DRV, buf, &len);
            if(read_res == HW_RES_OK && len == 1) {
                if(buf[0] == '>') {
                    SMSG("Sending data");
                    serial_clear_rx_buf(ESP8266_DRV);    /*Clear before receive the answer*/
                    serial_send_force(ESP8266_DRV, transf_buf, transf_size);
                    line_i = 0;
                    act_task_state ++;
                } else {
                    SWARN("Wrong resp.: %c", buf[0]);
                }
            } 
            break;
        case 3: /*Wait for "SEND OK"*/
            if(read_line() != false) {
                sprintf(buf, "Recv %d bytes\r\n", transf_size);
                if(strcmp(line_buf, "SEND OK\r\n") == 0) {
                    SMSG("Data sent");
                    recp = 0;
                    act_task_state ++;
                } else if (strcmp(buf, line_buf) == 0) {
                    SMSG(line_buf);
                } else {
                    SWARN("Wrong resp.: %s", line_buf);
                    act_task = ESP8266_TASK_NONE;
                    if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR, buf);
                }
            } 
            break;
         case 4: /*Wait for +IPD, */
            while(1) {
                len = 1;
                read_res = serial_rec(ESP8266_DRV, &transf_buf[recp], &len);
                if(read_res == HW_RES_OK && len == 1) {
                    recp ++;
                    transf_buf[recp] = '\0';
                    if(strcmp(transf_buf, "\r\n+IPD,") == 0) {
                        SMSG("+IPD ok");
                        recp = 0;
                        act_task_state ++;
                        break;
                    } else if(recp > 50) {
                        SWARN("No +IPD received");
                        act_task = ESP8266_TASK_NONE;
                        if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR, "Error while receiving answer");
                        break;
                    }
                } else {
                    break;
                }
            }
            break;
            
         case 5: /*Receiving data length*/
            while(1) {
                len = 1;
                read_res = serial_rec(ESP8266_DRV, &transf_buf[recp], &len);
                if(read_res == HW_RES_OK && len == 1) {
                    if(transf_buf[recp] == ':') {
                        transf_buf[recp + 1] = '\0'; 
                        sscanf(transf_buf, "%d", &transf_size);
                        SMSG("Rec. length: %d", transf_size);
                        if(transf_size > ESP8266_BUF_SIZE) {
                            SWARN("Too long data: %d", transf_size);
                            act_task = ESP8266_TASK_NONE;
                            if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR, "Too long data");
                        } else {
                            transf_buf[0] = (uint8_t) transf_size & 0xFF;    /*Save the data length*/ 
                            transf_buf[1] = (uint8_t) (transf_size >> 8) & 0xFF;
                            recp = 2;
                            act_task_state ++;
                            break;
                        }
                    } else if(recp > 8) {
                        SWARN("No answ. length received");
                        act_task = ESP8266_TASK_NONE;
                        if(act_cb != NULL) act_cb(ESP8266_STATE_ERROR, "Too long length info");
                        break;
                    }
                    recp ++;
                } else {
                    break;
                }
            }
            break;
         case 6: /*Receiving data*/
            while(1) {
                len = 1;
                read_res = serial_rec(ESP8266_DRV, &transf_buf[recp], &len); 
                if(read_res == HW_RES_OK && len == 1) {
                    if(recp >= transf_size + 1) {
                        SMSG("Data received");
                        act_task = ESP8266_TASK_NONE;
                        if(act_cb != NULL) act_cb(ESP8266_STATE_READY, (char *)transf_buf);
                        break;
                    } else {
                        recp++;
                    }
                } else {
                    break;
                }
            }
            break;
     }
     
    if(tick_elaps(act_timestamp) > ESP8266_TOUT_TCP_TRANSF) {
            act_cb(ESP8266_STATE_ERROR, "Timeout");
            act_task = ESP8266_TASK_NONE;
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
        res = serial_rec(ESP8266_DRV, &line_buf[line_i], &length);
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