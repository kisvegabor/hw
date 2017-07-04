/**
 * @file i2c.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_I2C != 0

#include <stddef.h>
#include "psp/psp_i2c.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the i2c modules
 */
void i2c_init(void)
{
    psp_i2c_init();
}
/**
 * 
 * @param adr
 * @param data_p
 * @param len
 * @return 
 */
hw_res_t i2c_send(i2c_t id, uint8_t adr, void * data_p, uint16_t len)
{
    hw_res_t res = HW_RES_OK;
    
    res = psp_i2c_start(id);
    
    if(res == HW_RES_OK) {
        res = psp_i2c_wr(id, (adr << 1) & 0xFE);
    }
    
    if(res == HW_RES_OK) {
        uint8_t * d8_p = data_p;  /*Easy to use uint8_t pointer to the data*/
        for(; len > 0; len--) {

            res = psp_i2c_wr(id, *d8_p);
            if(res != HW_RES_OK) {
                break;
            }
            d8_p ++;
        }
    }

    /*Uncoditionally send a stop condition*/
    psp_i2c_stop(id);
    
    return res;
}

/**
 * Read data via I2C.  
 * @param id I2C driver ID
 * @param adr I2C slave address (7 bit without R/W bit)
 * @param cmd pointer to a command to send before read (e.g. memory address in EEPROM) NULL if unused
 * @param cmd_len length of command n bytes
 * @param data_p pointer to a buffer to store the read data
 * @param read_len umber of bytes to read
 * @return HW_RES_OK or any error from hw_res_t
 */
hw_res_t i2c_read(i2c_t id, uint8_t adr, void * cmd, uint8_t cmd_len, void * data_p, uint16_t read_len)
{
    hw_res_t res = HW_RES_OK;
   
    /*Address the salve for write and send the command*/
    res = psp_i2c_start(id);
    
    /*If no command just read*/
    if(cmd == NULL) {
        if(res == HW_RES_OK) {
            res = psp_i2c_wr(id, (adr << 1) & 0xFE);   
        }

        /*Write the commands*/
        if(res == HW_RES_OK) {
            uint8_t * d8_p = cmd;  /*Easy to use uint8_t pointer to the data*/
            for(; cmd_len > 0; cmd_len--) {

                res = psp_i2c_wr(id, *d8_p);
                if(res != HW_RES_OK) {
                    break;
                }
                d8_p ++;
            }
        }

        /*Restart, address for read and begin the reading*/
        if(res == HW_RES_OK) {
            res = psp_i2c_restart(id);
        }
    }
    
    if(res == HW_RES_OK) {
        psp_i2c_wr(id, (adr << 1) | 0x01);
    }
    
    /*Ready all bytes except the last*/
    if(res == HW_RES_OK) {
        uint8_t * d8_p = data_p;
        bool ack;
        for(; read_len > 0; read_len --) {

            if(read_len > 1) {
                ack = true;
            } else {
                ack = false;
            }
            res = psp_i2c_rd(id, d8_p, ack);
            if(res != HW_RES_OK) {
                break;
            }
            d8_p ++;
        }
    }
    
    /*Unconditionally send a stop condition*/
    psp_i2c_stop(id);
    
    return res;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
