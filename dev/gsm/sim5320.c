/**
 * @file sim5230.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "sim5320.h"
#if USE_SIM5320 != 0

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "hw/dev/ui/log.h"
#include "hw/per/serial.h"
#include "hw/per/tick.h"
#include "misc/os/ptask.h"

/*********************
 *      DEFINES
 *********************/
#define SIM5320_TOUT_NETW_CON   10000       /*ms*/
#define SIM5320_TOUT_TCP_CON    30000       /*ms*/
#define SIM5320_TOUT_TCP_LEAVE  5000       /*ms*/
#define SIM5320_TOUT_TCP_TRANSF 10000       /*ms*/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum
{
    SIM5320_TASK_NONE = 0,
    SIM5320_TASK_NETW_CON,
    SIM5320_TASK_NETW_LEAVE,
    SIM5320_TASK_TCP_CON,
    SIM5320_TASK_TCP_LEAVE,
    SIM5320_TASK_TCP_TRANSF,
    SIM5320_TASK_NUM,
    SIM5320_TASK_INV = 0xFF
}sim5320_task_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void sim5320_handler(void * param);
static void netw_con_handler(void);
static void netw_leave_handler(void);
static void tcp_con_handler(void);
static void tcp_leave_handler(void);
static void tcp_transf_handler(void);

static sim5320_state_t read_line(void);


/**********************
 *  STATIC VARIABLES
 **********************/
static sim5320_task_t act_task = SIM5320_TASK_NONE;
static sim5320_cb_t act_cb = NULL;
static uint8_t act_task_state = 0;
static uint32_t act_timestamp = 0;
static char last_param1[64];
static char last_param2[64];
static bool com_ready = false;

static char transf_buf[SIM5320_BUF_SIZE];
static int transf_size;
static char line_buf[256];
static uint16_t line_i;

LOG_FN("sim5320");

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void sim5320_init(void)
{
    serial_set_baud(SIM5320_DRV, 115200);
    serial_send_force(SIM5320_DRV, "ATE0\r\n", SERIAL_SEND_STRING);
    tick_wait_ms(200);
    
    
    ptask_create(sim5320_handler, 100, PTASK_PRIO_MID, NULL);
}

sim5320_state_t sim5320_netw_con(const char * apn, sim5320_cb_t cb)
{
    SMSG("Network con...");
    
    if(act_task != SIM5320_TASK_NONE) {
        SWARN("Can not connect: busy");
        return SIM5320_STATE_BUSY;
    }
    
    strcpy(last_param1, apn);
    act_task_state = 0;
    act_timestamp = tick_get();
    act_cb = cb;
    serial_clear_rx_buf(SIM5320_DRV);
    line_i = 0;
    act_task = SIM5320_TASK_NETW_CON;
    
    return SIM5320_STATE_OK;
}

sim5320_state_t sim5320_netw_leave(sim5320_cb_t cb)
{
    SMSG("Network leave...");
    
    if(act_task != SIM5320_TASK_NONE) {
        SWARN("Can not leave: busy");
        return SIM5320_STATE_BUSY;
    }
    
    act_task_state = 0;
    act_timestamp = tick_get();
    act_cb = cb;
    serial_clear_rx_buf(SIM5320_DRV);
    line_i = 0;
    act_task = SIM5320_TASK_NETW_LEAVE;
    
    return SIM5320_STATE_OK;
}

sim5320_state_t sim5320_tcp_con(const char * ip, const char * port, sim5320_cb_t cb)
{
    SMSG("TCP con...");
    
    if(act_task != SIM5320_TASK_NONE) {
        SWARN("Can not connect: busy");
        return SIM5320_STATE_BUSY;
    }
    
    strcpy(last_param1, ip);
    strcpy(last_param2, port);
    act_task_state = 0;
    act_timestamp = tick_get();
    act_cb = cb;
    serial_clear_rx_buf(SIM5320_DRV);
    line_i = 0;
    act_task = SIM5320_TASK_TCP_CON;
    
    return SIM5320_STATE_OK;
    
}

sim5320_state_t sim5320_tcp_leave(sim5320_cb_t cb)
{
    
    SMSG("TCP leave...");
    
    if(act_task != SIM5320_TASK_NONE) {
        SWARN("Can not connect: busy");
        return SIM5320_STATE_BUSY;
    }
    
    act_task_state = 0;
    act_timestamp = tick_get();
    act_cb = cb;
    serial_clear_rx_buf(SIM5320_DRV);
    line_i = 0;
    act_task = SIM5320_TASK_TCP_LEAVE;
    
    return SIM5320_STATE_OK;
}

sim5320_state_t sim5320_tcp_transf(const void * data, uint16_t len, sim5320_cb_t cb)
{
    SMSG("TCP leave...");
    
    if(act_task != SIM5320_TASK_NONE) {
        SWARN("Can not connect: busy");
        return SIM5320_STATE_BUSY;
    }
    
    act_task_state = 0;
    act_timestamp = tick_get();
    act_cb = cb;
    serial_clear_rx_buf(SIM5320_DRV);
    line_i = 0;
    act_task = SIM5320_TASK_TCP_TRANSF;
    
    return SIM5320_STATE_OK;
}

bool sim5320_busy(void)
{
    return act_task == SIM5320_TASK_NONE ? false : true;
}


/**********************
 *   STATIC FUNCTIONS
 **********************/
static void sim5320_handler(void * param)
{
    if(act_task == SIM5320_TASK_NONE) {
        act_task_state = 0;
        
        if(com_ready != false) {
        }
        
        return;
    }
    else if(act_task == SIM5320_TASK_NETW_CON) {
        netw_con_handler();
    } else if(act_task == SIM5320_TASK_NETW_LEAVE) {
        netw_leave_handler(); 
    } else if(act_task == SIM5320_TASK_TCP_CON) {
        tcp_con_handler();
    } else if(act_task == SIM5320_TASK_TCP_LEAVE) {
        tcp_leave_handler();
    } else if(act_task == SIM5320_TASK_TCP_TRANSF) {
        tcp_transf_handler();
    }        
}


static void netw_con_handler(void)
{
    sim5320_state_t read_res;
    char buf[128];
    
     switch(act_task_state) {
        case 0:
            sprintf(buf,"AT+CGSOCKCONT=1,\"IP\",\"%s\"\r\n" , last_param1);
            serial_send_force(SIM5320_DRV, buf, SERIAL_SEND_STRING);
            act_task_state++;
            break;
        case 1:
            read_res = read_line();
            if(read_res == SIM5320_STATE_OK) {
                if (strcmp(line_buf, "OK\r\n") == 0) {
                    SMSG("Define socket PDP context: ok");
                    act_task_state++;
                } else {
                    SWARN("Wrong resp.: %s", line_buf);
                    act_task = SIM5320_TASK_NONE;
                    if(act_cb != NULL) act_cb(SIM5320_STATE_ERROR,  line_buf);
                }
            } 
            break;
        case 2:
            serial_send_force(SIM5320_DRV, "AT+CSOCKSETPN=1\r\n", SERIAL_SEND_STRING);
            act_task_state++;
            break;
        case 3:
            read_res = read_line();
            if(read_res == SIM5320_STATE_OK) {
                if (strcmp(line_buf, "OK\r\n") == 0) {
                    SMSG("PDP Context's profile: ok");
                    act_task_state++;
                } else {
                    SWARN("Wrong resp.: %s", line_buf);
                    act_task = SIM5320_TASK_NONE;
                    if(act_cb != NULL) act_cb(SIM5320_STATE_ERROR,  line_buf);
                }
            } 
            break;
        case 4:
            serial_send_force(SIM5320_DRV, "AT+NETOPEN=,,1\r\n", SERIAL_SEND_STRING);
            act_task_state++;
            break;
        case 5:
            read_res = read_line();
            if(read_res == SIM5320_STATE_OK) {
                if (strcmp(line_buf, "OK\r\n") == 0) {
                    SMSG("Connected to mobile data network");
                    act_task = SIM5320_TASK_NONE;
                    if(act_cb != NULL) act_cb(SIM5320_STATE_OK,  "");
                } else if (strcmp(line_buf, "+IP ERROR: Network is already opened\r\n") == 0) {
                    SMSG("Already connected to network");
                    act_task = SIM5320_TASK_NONE;
                    if(act_cb != NULL) act_cb(SIM5320_STATE_OK, "");
                } else if(strcmp(line_buf, "Network opened\r\n") == 0) {
                    SMSG("Network opened received");
                } else {
                    SWARN("Wrong resp.: %s", line_buf);
                    act_task = SIM5320_TASK_NONE;
                    if(act_cb != NULL) act_cb(SIM5320_STATE_ERROR,  line_buf);
                }
            } 
            break;
     }
     
     if(tick_elaps(act_timestamp) > SIM5320_TOUT_NETW_CON) {
        SWARN("Timeouted");
        act_task = SIM5320_TASK_NONE;
        act_cb(SIM5320_STATE_ERROR, "Timeout");
    }
}

static void netw_leave_handler(void)
{ 
    sim5320_state_t read_res;
    
     switch(act_task_state) {
         case 0:
            serial_send_force(SIM5320_DRV, "AT+CIPCLOSE=0\r\n", SERIAL_SEND_STRING);
            act_task_state++;
            break;
         case 1:
            read_res = read_line();
            if(read_res == SIM5320_STATE_OK) {
                if(strcmp(line_buf, "OK\r\n") == 0) {
                    SMSG("TCP Connections closed");
                    act_task_state ++;
                } else {
                    SWARN("TCP connection close error: %s (continue network leave)", line_buf);
                    act_task_state ++;
                }
            }
            break;
        case 2:
            serial_clear_rx_buf(SIM5320_DRV);
            serial_send_force(SIM5320_DRV, "AT+NETCLOSE\r\n", SERIAL_SEND_STRING);
            act_task_state++;
            break;
         case 3:              
            read_res = read_line();
            if(read_res == SIM5320_STATE_OK) {
                if(strcmp(line_buf, "OK\r\n") == 0) {
                    SMSG("Network closed");
                    act_task = SIM5320_TASK_NONE;
                    if(act_cb != NULL) act_cb(SIM5320_STATE_OK, "");
                } else if(strcmp(line_buf, "Network closed\r\n") == 0) { 
                    SMSG("Network closed received");
                } else if(strcmp(line_buf, "+IP ERROR: Network is already closed") == 0) {
                    SMSG("Network already closed");
                } else {
                    SWARN("Wrong resp: %s", line_buf);
                    act_task = SIM5320_TASK_NONE;
                    if(act_cb != NULL) act_cb(SIM5320_STATE_ERROR, line_buf); 
                }
            }
            break;
     } 
     if(tick_elaps(act_timestamp) > SIM5320_TOUT_TCP_LEAVE) {
        SWARN("Timeout");
        act_task = SIM5320_TASK_NONE;
        act_cb(SIM5320_STATE_ERROR, "Timeout");
    }
}

static void tcp_con_handler(void)
{
    sim5320_state_t read_res;
    char buf[128];
    
     switch(act_task_state) {
        case 0:
            sprintf(buf,"AT+CIPOPEN=0,\"TCP\",\"%s\",%s\r\n", last_param1, last_param2);
            serial_send_force(SIM5320_DRV, buf, SERIAL_SEND_STRING);
            act_task_state++;
            break;
        case 1:
            read_res = read_line();
            if(read_res == SIM5320_STATE_OK) {
                if (strcmp(line_buf, "OK\r\n") == 0) {
                    act_task = SIM5320_TASK_NONE;
                    SMSG("TCP connected");
                    if(act_cb != NULL) act_cb(SIM5320_STATE_OK, "");
                } else if (strcmp(line_buf, "Connect ok\r\n") == 0) {
                    SMSG("Conect OK received");
                    if(act_cb != NULL) act_cb(SIM5320_STATE_OK, "");
                } else {
                    SWARN("Wrong resp.: %s", line_buf);
                    act_task = SIM5320_TASK_NONE;
                    if(act_cb != NULL) act_cb(SIM5320_STATE_ERROR,  line_buf);
                }
            } 
            break;
     }
     
     if(tick_elaps(act_timestamp) > SIM5320_TOUT_TCP_CON) {
        SWARN("Timeouted");
        act_task = SIM5320_TASK_NONE;
        act_cb(SIM5320_STATE_ERROR, "Timeout");
    }
}

static void tcp_leave_handler(void)
{ 
    sim5320_state_t read_res;
    
     switch(act_task_state) {
        case 0:
            serial_send_force(SIM5320_DRV, "AT+CIPCLOSE=0\r\n", SERIAL_SEND_STRING);
            act_task_state++;
            break;
         case 2:
            read_res = read_line();
            if(read_res == SIM5320_STATE_OK) {
                if(strcmp(line_buf, "OK\r\n") == 0) {
                    SMSG("TCP Connection closed");
                    act_task = SIM5320_TASK_NONE;
                    if(act_cb != NULL) act_cb(SIM5320_STATE_OK, "");
                } else {
                    SWARN("Wrong resp: %s", line_buf);
                    act_task = SIM5320_TASK_NONE;
                    if(act_cb != NULL) act_cb(SIM5320_STATE_ERROR, line_buf); 
                }
            }
            break;
     } 
     if(tick_elaps(act_timestamp) > SIM5320_TOUT_TCP_LEAVE) {
        SWARN("Timeout");
        act_task = SIM5320_TASK_NONE;
        act_cb(SIM5320_STATE_ERROR, "Timeout");
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
            serial_send_force(SIM5320_DRV, buf, SERIAL_SEND_STRING);
            act_task_state++;
            break;
            
        case 1: /*Wait for OK*/
            if(read_line() == SIM5320_STATE_OK) {
                if(strcmp(line_buf, "OK\r\n") == 0) {
                    SMSG("Data send OK received");
                    act_task_state ++;
                } else {
                    SWARN("Wrong resp.: %s", line_buf);
                    act_task = SIM5320_TASK_NONE;
                    if(act_cb != NULL) act_cb(SIM5320_STATE_ERROR, line_buf);
                }
            } 
            break;
            
            
         case 2: /*Wait for ">" and send data*/
            len = 1;
            read_res = serial_rec(SIM5320_DRV, buf, &len);
            if(read_res == HW_RES_OK && len == 1) {
                if(buf[0] == '>') {
                    SMSG("Sending data");
                    serial_clear_rx_buf(SIM5320_DRV);    /*Clear before receive the answer*/
                    serial_send_force(SIM5320_DRV, transf_buf, transf_size);
                    line_i = 0;
                    act_task_state ++;
                } else {
                    SWARN("Wrong resp.: %c", buf[0]);
                }
            } 
            break;
        case 3: /*Wait for "SEND OK"*/
            if(read_line() == SIM5320_STATE_OK) {
                sprintf(buf, "Recv %d bytes\r\n", transf_size);
                if(strcmp(line_buf, "Send ok\r\n") == 0) {
                    SMSG("Data sent");
                    recp = 0;
                    act_task_state ++;
                } else if (strcmp(buf, line_buf) == 0) {
                    SMSG(line_buf);
                } else {
                    SWARN("Wrong resp.: %s", line_buf);
                    act_task = SIM5320_TASK_NONE;
                    if(act_cb != NULL) act_cb(SIM5320_STATE_ERROR, buf);
                }
            } 
            break;
         case 4: /*Wait for +IPD */
            while(1) {
                len = 1;
                read_res = serial_rec(SIM5320_DRV, &transf_buf[recp], &len);
                if(read_res == HW_RES_OK && len == 1) {
                    recp ++;
                    transf_buf[recp] = '\0';
                    if(strcmp(transf_buf, "\r\n+IPD") == 0) {
                        SMSG("+IPD ok");
                        recp = 0;
                        act_task_state ++;
                        break;
                    } else if(recp > 50) {
                        SWARN("No +IPD received");
                        act_task = SIM5320_TASK_NONE;
                        if(act_cb != NULL) act_cb(SIM5320_STATE_ERROR, "Error while receiving answer");
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
                read_res = serial_rec(SIM5320_DRV, &transf_buf[recp], &len);
                if(read_res == HW_RES_OK && len == 1) {
                    if(transf_buf[recp] == ':') {
                        transf_buf[recp + 1] = '\0'; 
                        sscanf(transf_buf, "%d", &transf_size);
                        SMSG("Rec. length: %d", transf_size);
                        if(transf_size > SIM5320_BUF_SIZE) {
                            SWARN("Too long data: %d", transf_size);
                            act_task = SIM5320_TASK_NONE;
                            if(act_cb != NULL) act_cb(SIM5320_STATE_ERROR, "Too long data");
                        } else {
                            transf_buf[0] = (uint8_t) transf_size & 0xFF;    /*Save the data length*/ 
                            transf_buf[1] = (uint8_t) (transf_size >> 8) & 0xFF;
                            recp = 2;
                            act_task_state ++;
                            break;
                        }
                    } else if(recp > 8) {
                        SWARN("No answ. length received");
                        act_task = SIM5320_TASK_NONE;
                        if(act_cb != NULL) act_cb(SIM5320_STATE_ERROR, "Too long length info");
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
                read_res = serial_rec(SIM5320_DRV, &transf_buf[recp], &len); 
                if(read_res == HW_RES_OK && len == 1) {
                    if(recp >= transf_size + 1) {
                        SMSG("Data received");
                        act_task = SIM5320_TASK_NONE;
                        if(act_cb != NULL) act_cb(SIM5320_STATE_OK, (char *)transf_buf);
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
     
    if(tick_elaps(act_timestamp) > SIM5320_TOUT_TCP_TRANSF) {
            act_cb(SIM5320_STATE_ERROR, "Timeout");
            act_task = SIM5320_TASK_NONE;
    }
}
    
static sim5320_state_t read_line(void)
{   
    hw_res_t res;
    int32_t length;
    while(line_i < sizeof(line_buf)) {
        length = 1;
        res = serial_rec(SIM5320_DRV, &line_buf[line_i], &length);
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
#if SIM5320_LOG_REC_LINES != 0
                        SMSG("Line received: %s", line_buf);
#endif
                        return SIM5320_STATE_OK;
                    }
                }
            }
        } else {
            return SIM5320_STATE_BUSY;
        }
    }
    
    if(line_i == sizeof(line_buf)) {
        SWARN("Overflow");
        line_i = 0;
    }
    
    return SIM5320_STATE_BUSY;
}

#endif  /*USE_SIM5320 != 0*/