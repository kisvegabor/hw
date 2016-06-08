/**
@file serial.c 
 */

/***********************
 *       INCLUDES
 ***********************/
#include "hw_conf.h"
#if USE_SERIAL != 0

#include <stddef.h>
#include <string.h>
#include "serial.h"
#include "misc/os/tick.h"
#include "misc/mem/fifo.h"

/***********************
 *       DEFINES
 ***********************/

/***********************
 *       TYPEDEFS
 ***********************/

/***********************
 *   GLOBAL VARIABLES
 ***********************/

/***********************
 *   STATIC VARIABLES
 ***********************/

/***********************
 *   GLOBAL PROTOTYPES
 ***********************/

/***********************
 *   STATIC PROTOTYPES
 ***********************/

/***********************
 *   GLOBAL FUNCTIONS
 ***********************/
/**
 * Init an SERIAL modules
 * @param modul_id the id of an SERIAL modul
 * @param baud the desired baud rate
 * @return HW_RES_OK or error
 */
void serial_init(void)
{
    psp_serial_init();
    
}

/**
 * Send data on SERIAL. (Background send)
 * Buffered, so just push the data into the buffer, but if the buffer is full 
 * the remeining data will not be sent (see 'length') 
 * @param id the id of an SERIAL modul
 * @param tx_buf pointer to a buffer where the data to send is stored
 * @param length before call: the length of tx_buf in bytes; (SERIAL_SEND_STRING can be used) 
 *               after call: the sent number of bytes
 *              
 * @return HW_RES_OK or error
 */
hw_res_t serial_send(serial_t id, const void * tx_buf, int32_t * length)
{
    hw_res_t res = HW_RES_OK;
    
    const uint8_t * buf8 = tx_buf;
    if(*length == SERIAL_SEND_STRING) *length = strlen(tx_buf);
    
    uint32_t i;
    for(i = 0; i < *length; i++) {
        res = psp_serial_wr(id, buf8[i]);

        /*Return if any error ocurred*/
        if(res != HW_RES_OK) {
            break;
        }
    }
    
    /*Set the sent number of bytes*/
    *length = i;
    
    //Return with the result
    return res;
}

/**
 * Send data on SERIAL. (Blocking send)
 * Buffer all possible data but wait here until every byte is not buffered
 * @param modul_id the id of an SERIAL modul
 * @param tx_buf pointer to a buffer where the data to send is stored
 * @param length the length of tx_buf in bytes (SERIAL_SEND_STRING can  be used)
 * @return HW_RES_OK or error
 */
hw_res_t serial_send_force(serial_t id, const void * tx_buf, int32_t length)
{
    hw_res_t res = HW_RES_OK;
    
    const uint8_t * buf8 = tx_buf;
    uint32_t i;
    if(length == SERIAL_SEND_STRING) length = strlen(tx_buf);

    for(i = 0; i < length; i++) {
        do {
            res = psp_serial_wr(id, buf8[i]);
            if(res == HW_RES_FULL) {
                tick_wait_ms(1);
            }
        } while(res == HW_RES_FULL);
        
        
        if(res != HW_RES_OK) {
            break;
        }
    }
    
    //Return with the result
    return res;
}

/*
 * Receive data from SERIAL
 * @param id the id of an SERIAL modul
 * @param rx_buf the received bytes will be stored here
 * @param length before call: how many bytes should be received, 
 *               after call: the number of real received bytes
 * @return HW_RES_OK or error
 */
hw_res_t serial_rec(serial_t id, void * rx_buf, uint32_t * length)
{
    hw_res_t res = HW_RES_OK;
    
    uint8_t * buf8 = rx_buf;
    uint32_t i;
    hw_res_t psp_res;

    for(i = 0; i < *length; i++) {
        psp_res = psp_serial_rd(id, &buf8[i]);

        /*Return if any error occurred*/
        if(psp_res != HW_RES_OK) {
            break;
        }
    }
    
    /*Set the received number of bytes*/
    *length = i;
    
    //Return with the result
    return res;
}


/*
 * Receive data from SERIAL (Blocking)
 * Wait here until 'length' bytes data can not be read
 * @param id the id of an SERIAL modul
 * @param rx_buf the received bytes will be stored here
 * @param length how many bytes should be received
 * @return HW_RES_OK or error
 */
hw_res_t serial_rec_force(serial_t id, void * rx_buf, uint32_t length)
{
     hw_res_t res = HW_RES_OK;
   
    uint8_t * buf8 = rx_buf;
    uint32_t i = 0;

    while(i < length) {
        res = psp_serial_rd(id, &buf8[i]);

        /*Check the return value*/
        if (res == HW_RES_OK) i++;
        else if (res == HW_RES_EMPTY)  tick_wait_ms(1);
        else  break;
    }
    //Return with the result
    return res;
}

hw_res_t serial_set_baud(serial_t id, uint32_t baud)
{
    hw_res_t res = HW_RES_OK;
    
    res = psp_serial_set_baud(id, baud);
    
    return res;
}

hw_res_t serial_clear_rx_buf(serial_t id) 
{
    hw_res_t res = HW_RES_OK;
    
    res = psp_serial_clear_rx_buf(id);
    
    return res;
}

/**
 * Estimate the required time of data sending
 * @param byte_num the number of bytes to send
 * @param baud the baud rate of the communication
 * @return the estimated send time in ms 
 */
uint32_t serial_get_send_time(uint32_t byte_num, uint32_t baud)
{
    return (uint32_t)((uint32_t) byte_num * 10 * 1000) / baud;
}

/***********************
 *   STATIC FUNCTIONS
 ***********************/

#endif
