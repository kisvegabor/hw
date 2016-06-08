/**
 * @file btm41x.c
 * 
 */
#include "hw_conf.h"
#if USE_BTM41X != 0

#include "btm41x.h"
#include <string.h>
#include <stdio.h>
#include "misc/os/tick.h"
#include "misc/os/log.h"
#include "hw/per/serial.h"
#include "hw/per/io.h"
#include "misc/os/ptask.h"


/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/
#define BTM41X_STARTUP_TIME 1000    /*750 ms in the datasheet*/
#define BTM41X_ANSW_TOUT 1000         /*ms*/
#define BTM41X_CONNECT_TOUT 2          /*ms*/
#define BTM41X_MANAGER_TASK_PERIOD  1000       /*ms*/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void bt41x_manager(void);
static bool btm41x_xfer_cmd(const char * cmd_p, const char * answ_p);
static bool btm41x_send_cmd(const char * cmd_p);
static bool btm41x_wait_for(const char * answ_p, uint16_t tout);

/**********************
 *  STATIC VARIABLES
 **********************/
LOG_FN("bt41x.c");
static btm41x_state_t state = BTM41X_STATE_INIT_ERR;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the Bluetooth module 
 * @return HW_RES_OK or any error from hw_res_t enum
 */
hw_res_t btm41x_init(void)
{   
    io_set_pin_dir(BTM41X_RST_PORT, BTM41X_RST_PIN, IO_DIR_OUT);
    
    /*Reset*/
    io_set_pin(BTM41X_RST_PORT, BTM41X_RST_PIN, 1);
    tick_wait_ms(50);
    io_set_pin(BTM41X_RST_PORT, BTM41X_RST_PIN, 0);
    tick_wait_ms(50);
    io_set_pin(BTM41X_RST_PORT, BTM41X_RST_PIN, 1);
    tick_wait_ms(BTM41X_STARTUP_TIME);
    
    serial_set_baud(BTM41X_SERIAL_DRV, 9600);
    
    if(btm41x_xfer_cmd("AT", "OK\r\n") == false) {
        SWARN("Not 9600 Baud, try 460800");
        
        tick_wait_ms(100);
        serial_set_baud(BTM41X_SERIAL_DRV, 460800);
        btm41x_send_cmd("");    /*Dummy command*/
        tick_wait_ms(100);
        serial_clear_rx_buf(BTM41X_SERIAL_DRV);
        if(btm41x_xfer_cmd("AT", "OK\r\n") == false) {
            SERR("Not 460800 Baud, exit");
            return HW_RES_NO_ACK;
        }
        
    }  else {
         /*Enable higher bauds*/
        if(btm41x_xfer_cmd("ATS525=0", "OK\r\n") == false) {
        SERR("High baud enable failed");
            return HW_RES_NO_ACK;
        }
        
        /*Set high baud*/
        if(btm41x_send_cmd("ATS521=460800") == false) {
        SERR("High baud set failed");
            return HW_RES_NO_ACK;
        }
        tick_wait_ms(100);
        serial_set_baud(BTM41X_SERIAL_DRV, 460800);
        btm41x_send_cmd("");    /*Dummy command*/
        tick_wait_ms(100);
        
        serial_clear_rx_buf(BTM41X_SERIAL_DRV);
        if(btm41x_xfer_cmd("AT", "OK\r\n") == false) {
            SERR("No answer with the new baud rate");
            return HW_RES_NO_ACK;
        }  
    }
    
    
    if(btm41x_xfer_cmd("ATS321=3", "OK\r\n") == false) {
        SERR("IO cap. set failed");
            return HW_RES_NO_ACK;
    }
    
    if(btm41x_xfer_cmd("ATS512=4", "OK\r\n") == false) {
        SERR("Failed to make discoverable and connectable");
            return HW_RES_NO_ACK;
    }
    
    if(btm41x_xfer_cmd("ATS0=1", "OK\r\n") == false) {
        SERR("Enable auto answer after 1 ring failed");
            return HW_RES_NO_ACK;
    }
    
    if(btm41x_xfer_cmd("AT+BTK=\"\"", "OK\r\n") == false) {
        SERR("Failed to set pin code");
            return HW_RES_NO_ACK;
    }
    
    if(btm41x_xfer_cmd("AT&W", "OK\r\n") == false) {
        SERR("Save data failed");
            return HW_RES_NO_ACK;
    }
    
    if(btm41x_xfer_cmd("ATZ", "OK\r\n") == false) {
        SERR("SW reset failed");
            return HW_RES_NO_ACK;
    }
    
    char buf[256];
    sprintf(buf, "AT+BTF=\"%s\"", BTM41X_NAME);
    if(btm41x_xfer_cmd(buf, "OK") == false) {
        SERR("Failed to set default name");
        return HW_RES_NO_ACK;
    }
    
    ptask_create(bt41x_manager, BTM41X_MANAGER_TASK_PERIOD, PTASK_PRIO_LOW);
  
    state = BTM41X_STATE_NOT_CONNECTED;
    
    return HW_RES_OK;
}

/**
 * Give the state of the Bluetooth module
 * @return An element of btm41x_t enum according to the recent state
 */
btm41x_state_t btm41x_state(void)
{
    return state;
}

/**
 * Receive data through Bluetooth
 * @param buf buffer for the received data
 * @param len number of bytes to receive, after return the real received number of bytes
 * @return HW_RES_OK or any error from hw_res_t enum
 */
hw_res_t btm41x_rec(void * buf, uint32_t * len)
{
    if(state != BTM41X_STATE_CONNECTED){
        SWARN("No connection");
        *len = 0;
        return HW_RES_NOT_RDY;
    }
    
    return serial_rec(BTM41X_SERIAL_DRV, buf, len);
}

/**
 * Send data through Bluetooth
 * @param buf data to send
 * @param len length of data in bytes
 * @return HW_RES_OK or any error from hw_res_t enum
 */
hw_res_t btm41x_send(void * buf, int32_t len)
{
    if(state != BTM41X_STATE_CONNECTED){
        SWARN("No connection");
        return HW_RES_NOT_RDY;
    }
    return serial_send(BTM41X_SERIAL_DRV, buf, &len);
}


/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Runs periodically and waits for a connection
 */
static void bt41x_manager(void)
{
    if(state == BTM41X_STATE_CONNECTED)return;
    
    if(btm41x_wait_for("CONNECT", BTM41X_CONNECT_TOUT) != false) {
        SMSG("Connection established!");
        state = BTM41X_STATE_CONNECTED;
    } else {
        SMSG("Waiting for connection");
    }        
}

/**
 * Send a command and waits for its answer
 * @param cmd_p the command (a string, eg. "AT")
 * @param answ_p the expected answer (a string, eg. "OK")
 * @return true: the command is sent the expected answer si arrived
 */
static bool btm41x_xfer_cmd(const char * cmd_p, const char * answ_p)
{
    if(btm41x_send_cmd(cmd_p) == false) {
        SERR("Cmd can not be sent %s", cmd_p);
        return false;
    }
    
    if(btm41x_wait_for(answ_p, BTM41X_ANSW_TOUT) == false) {
        SERR("%s not received for %s",answ_p, cmd_p);
        return false;
    }
    
    SMSG("cmd: %s answ: %s", cmd_p, answ_p);
    
    return true;
    
}

/**
 * Send a command
 * @param cmd_p the command (a string, e.g. "AT")
 * @return true: the command is successfully sent
 */
static bool btm41x_send_cmd(const char * cmd_p)
{
    hw_res_t res;
    res = serial_send_force(BTM41X_SERIAL_DRV, cmd_p, SERIAL_SEND_STRING);
    if(res != HW_RES_OK) return false;
    
    res = serial_send_force(BTM41X_SERIAL_DRV, "\r\n", SERIAL_SEND_STRING);
    if(res != HW_RES_OK) return false;
        
    return true;
}

/**
 * Wait for a data (typically an answer) until timeout
 * @param answ_p ta string to wait for (eg. "OK")
 * @param tout timeout in milliseconds (resets after a character is received)
 * @return true: the data is arrived
 */
static bool btm41x_wait_for(const char * answ_p, uint16_t tout)
{
    char buf[64];
    uint8_t i = 0;
    uint32_t rec_len;
    uint32_t answ_len = strlen(answ_p);
    uint32_t start_time = tick_get();
    
    while(1) {
        rec_len = 1;
        serial_rec(BTM41X_SERIAL_DRV, &buf[i], &rec_len);
        if(rec_len == 1) {
           //serial_send_force(LOG_SERIAL_DRV, &buf[i], 1);
            start_time = tick_get();    /*Reset timeout if a char received*/
            if(i == sizeof(buf)) return false;
   
            if(i >= answ_len) {
                buf[i + 1] = '\0'; /*Convert to string*/
                if(strcmp(&buf[i - (answ_len - 1)], answ_p) == 0){
                    return true;
                }
            }
            i++;
        }
        
        if(tick_elaps(start_time) > tout) return false;      
    }   
}

#endif /*USE_BTM41x != 0*/