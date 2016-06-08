/**
 * @file spim.c
 * It is a special software SPI which has got 1 SDO and max. 4 SDI  
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_SPIM != 0

#include <stddef.h>
#include "spi.h"
#include "io.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void spim_byte_xchg(uint8_t tx, uint8_t * rx_buf);

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
 * Initialize the SPI Multi module
 * @return HW_RES_OK or any error from hw_res_t
 */
hw_res_t spim_init(void)
{
    /*Init the CS pin*/
    io_set_pin_dir(SPIM_CS_PORT, SPIM_CS_PIN, IO_DIR_OUT);
    io_set_pin(SPIM_CS_PORT, SPIM_CS_PIN, 1);    
    /*Init the SCK pin*/
    io_set_pin_dir(SPIM_SCK_PORT, SPIM_SCK_PIN, IO_DIR_OUT);
    io_set_pin(SPIM_SCK_PORT, SPIM_SCK_PIN, 1);  
    /*Init the SDO pin*/
    io_set_pin_dir(SPIM_SDO_PORT, SPIM_SDO_PIN, IO_DIR_OUT);
    io_set_pin(SPIM_SDO_PORT, SPIM_SDO_PIN, 1);  
    /*Init the SDO pin*/
    io_set_pin_dir(SPIM_SDI1_PORT, SPIM_SDI1_PIN, IO_DIR_IN);
    io_set_pin_dir(SPIM_SDI2_PORT, SPIM_SDI2_PIN, IO_DIR_IN);
    io_set_pin_dir(SPIM_SDI3_PORT, SPIM_SDI3_PIN, IO_DIR_IN);
    io_set_pin_dir(SPIM_SDI4_PORT, SPIM_SDI4_PIN, IO_DIR_IN);
    
    return HW_RES_OK;
}

/**
 * Pull down the Chip Select line
 * @return HW_RES_OK or any error from hw_res_t
 */
hw_res_t spim_cs_en(void)
{
    io_set_pin(SPIM_CS_PORT, SPIM_CS_PIN, 0);
    
    return HW_RES_OK;
}

/**
 * Release the Chip Select line
 * @return HW_RES_OK or any error from hw_res_t
 */
hw_res_t spim_cs_dis(void)
{
    io_set_pin(SPIM_CS_PORT, SPIM_CS_PIN, 1);
    
    return HW_RES_OK;
}

/**
 * Make a transfer 
 * @param tx_buf bytes to send
 * @param rx_buf received bytes (SPIM_IN_NUM * length bytes will be received)
 *                the order is: dev1 byte1, dev2 byte1, dev3 byte1, dev4 byte 1, dev1 byte 2 ...
 * @param length length of tx_buf in bytes (number of bytes to send)
 * @return HW_RES_OK 
 */
hw_res_t spim_xchg(void * tx_buf, void * rx_buf, uint32_t length)
{
    hw_res_t res = HW_RES_OK;
 
    /*If SW SPI use the sw_xchg function */
    uint8_t * rx_buf8 = rx_buf;
    uint8_t * tx_buf8 = tx_buf;
    uint8_t tx_act;
    
    uint32_t i;
    for(i = 0; i < length; i++)
    {
        tx_act = tx_buf8[i];
        spim_byte_xchg(tx_act, &rx_buf8[i * SPIM_IN_NUM]);
    }
    
    return res;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Send one byte an receive SPIM_IN_NUM
 * @param tx byte to send
 * @param rx_buf the received bytes
 */
static void spim_byte_xchg(uint8_t tx, uint8_t * rx_buf)
{
    uint8_t mask = 0b10000000;
    uint8_t i;
    uint8_t act_rec;

    /*CLK = 0*/    
    io_set_pin(SPIM_SCK_PORT, SPIM_SCK_PIN, 0);

    for(i = 0; i < 8; i ++) {   
        if(tx & mask)  io_set_pin(SPIM_SDO_PORT, SPIM_SDO_PIN, 1);
        else io_set_pin(SPIM_SDO_PORT, SPIM_SDO_PIN, 0);
        
        /*SCK = 1*/
        io_set_pin(SPIM_SCK_PORT, SPIM_SCK_PIN, 1);
        
        act_rec = 0;
        
        /*Read SDI1*/
#if SPIM_IN_NUM >= 1
        rx_buf[act_rec] = rx_buf[act_rec] << 1;
        if(io_get_pin(SPIM_SDI1_PORT, SPIM_SDI1_PIN) != 0)  rx_buf[act_rec]++; 
        act_rec ++;
#endif
        
        /*Read SDI2*/
#if SPIM_IN_NUM >= 2
        rx_buf[act_rec] = rx_buf[act_rec] << 1;
        if(io_get_pin(SPIM_SDI2_PORT, SPIM_SDI2_PIN) != 0) rx_buf[act_rec]++; 
        act_rec ++;
#endif
        
        /*Read SDI3*/
#if SPIM_IN_NUM >= 3
        rx_buf[act_rec] = rx_buf[act_rec] << 1;
        if(io_get_pin(SPIM_SDI3_PORT, SPIM_SDI3_PIN) != 0) rx_buf[act_rec]++; 
        act_rec ++;
#endif
        
        /*Read SDI1*/
#if SPIM_IN_NUM >= 4
        rx_buf[act_rec] = rx_buf[act_rec] << 1;
        if(io_get_pin(SPIM_SDI4_PORT, SPIM_SDI4_PIN) != 0) rx_buf[act_rec]++; 
        act_rec ++;
#endif
        mask=mask>>1;
        
        /*SCK = 0*/
        io_set_pin(SPIM_SCK_PORT, SPIM_SCK_PIN, 0);

    }
}

#endif
