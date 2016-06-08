/**
 * @file spi.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_SPI != 0

#include <stddef.h>
#include "spi.h"
#include "io.h"
#include "psp/psp_spi.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct
{
    io_port_t port;
    io_port_t pin;
}spi_cs_pin_t;


/**********************
 *  STATIC PROTOTYPES
 **********************/
static uint8_t spisw_byte_xchg(uint8_t tx);

/**********************
 *  STATIC VARIABLES
 **********************/
static const spi_cs_pin_t spi_cs[] = 
{
/*SPI1_CS1*/    {SPI1_CS1_PORT,  SPI1_CS1_PIN},
/*SPI1_CS2*/    {SPI1_CS2_PORT,  SPI1_CS2_PIN},
/*SPI1_CS3*/    {SPI1_CS3_PORT,  SPI1_CS3_PIN},
/*SPI1_CS4*/    {SPI1_CS4_PORT,  SPI1_CS4_PIN},
/*SPI2_CS1*/    {SPI2_CS1_PORT,  SPI2_CS1_PIN},
/*SPI2_CS2*/    {SPI2_CS2_PORT,  SPI2_CS2_PIN},
/*SPI2_CS3*/    {SPI2_CS3_PORT,  SPI2_CS3_PIN},
/*SPI2_CS4*/    {SPI2_CS4_PORT,  SPI2_CS4_PIN},
/*SPI3_CS1*/    {SPI3_CS1_PORT,  SPI3_CS1_PIN},
/*SPI3_CS2*/    {SPI3_CS2_PORT,  SPI3_CS2_PIN},
/*SPI3_CS3*/    {SPI3_CS3_PORT,  SPI3_CS3_PIN},
/*SPI3_CS4*/    {SPI3_CS4_PORT,  SPI3_CS4_PIN},
/*SPI4_CS1*/    {SPI4_CS1_PORT,  SPI4_CS1_PIN},
/*SPI4_CS2*/    {SPI4_CS2_PORT,  SPI4_CS2_PIN},
/*SPI4_CS3*/    {SPI4_CS3_PORT,  SPI4_CS3_PIN},
/*SPI4_CS4*/    {SPI4_CS4_PORT,  SPI4_CS4_PIN},
/*SPI5_CS1*/    {SPI5_CS1_PORT,  SPI5_CS1_PIN},
/*SPI5_CS2*/    {SPI5_CS2_PORT,  SPI5_CS2_PIN},
/*SPI5_CS3*/    {SPI5_CS3_PORT,  SPI5_CS3_PIN},
/*SPI5_CS4*/    {SPI5_CS4_PORT,  SPI5_CS4_PIN},
/*SPISW_CS1*/   {SPISW_CS1_PORT, SPISW_CS1_PIN},
/*SPISW_CS2*/   {SPISW_CS2_PORT, SPISW_CS2_PIN},
/*SPISW_CS3*/   {SPISW_CS3_PORT, SPISW_CS3_PIN},
/*SPISW_CS4*/   {SPISW_CS4_PORT, SPISW_CS4_PIN},
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
 * Initialize the SPI
 * @return HW_RES_OK or any error from hw_res_t
 */
void spi_init(void)
{
    /*CS init*/
    spi_t i;
    for(i = HW_SPI1_CS1; i < HW_SPI_NUM; i++ ) {
        io_set_pin_dir(spi_cs[i].port, spi_cs[i].pin, IO_DIR_OUT);
        spi_cs_dis(i);
    }
    
    /*SW SPI init*/
    io_set_pin_dir(SPISW_SCK_PORT, SPISW_SCK_PIN, IO_DIR_OUT);
    io_set_pin(SPISW_SCK_PORT, SPISW_SCK_PIN, 0);
    
    io_set_pin_dir(SPISW_SDO_PORT, SPISW_SDO_PIN, IO_DIR_OUT);
    io_set_pin(SPISW_SDO_PORT, SPISW_SDO_PIN, 0);   
    
    io_set_pin_dir(SPISW_SDI_PORT, SPISW_SDI_PIN, IO_DIR_IN);
    
    /*HW SPI init*/
    psp_spi_init(); 
    
}

/**
 * Pull down the SPI Chip Select
 * @param spi the ID of an SPI module (HW_SPIx_CSy)
 */
void spi_cs_en(spi_t spi)
{
    io_set_pin(spi_cs[spi].port, spi_cs[spi].pin, 0);
}

/**
 * Release the Chip Select
 * @param spi the ID of an SPI module (HW_SPIx_CSy)
 */
void spi_cs_dis(spi_t spi)
{
    io_set_pin(spi_cs[spi].port, spi_cs[spi].pin, 1);
}

/**
 * Make an SPI transfer with a given number of bytes
 * @param spi the ID of an SPI module (HW_SPIx_CSy)
 * @param tx_buf buffer with the bytes to send
 * @param rx_buf buffer for the received bytes
 * @param length number of bytes o send
 */
void spi_xchg(spi_t spi, const void * tx_buf, void * rx_buf, uint32_t length)
{
    const uint8_t * tx8 = tx_buf;
    uint8_t * rx8 = rx_buf;
    uint8_t rec;
    uint32_t i;
    
    if(spi < HW_SPISW_CS1) {
        spi = spi >> SPI_CS_SHIFT; /*Convert to spi_hw_t*/
        psp_spi_xchg(spi, tx_buf, rx_buf, length);
    } else if (spi < HW_SPI_NUM) { 
        for(i = 0; i < length; i++) {
            if(tx8 != NULL) rec = spisw_byte_xchg(tx8[i]);
            else rec = spisw_byte_xchg(0xFF);
            
            if(rx8 != NULL) rx8[i] = rec; 
        }
    }
}

/**
 * Set a new baud rate for an SPI module
 * @param spi the ID of an SPI module (HW_SPIx_CSy)
 * @param baud the new baud rate (SPI_BAUD_MAX for the grates possible baud)
 */
void spi_set_baud(spi_t spi, uint32_t baud)
{
    if(spi < HW_SPISW_CS1) {
        spi = spi >> SPI_CS_SHIFT; /*Convert to spi_hw_t*/
        psp_spi_set_baud(spi, baud);
    }   
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Send a byte via software SPI
 * @param tx byte to send
 * @return received byte
 */
static uint8_t spisw_byte_xchg(uint8_t tx)
{
    uint8_t mask = 0b10000000;
    uint8_t rec = 0;
    uint8_t i;

    /*CLK = 0*/    
    io_set_pin(SPISW_SCK_PORT, SPISW_SCK_PIN, 0);

    for(i = 0; i < 8; i ++) {
        rec = rec << 1;
        
        if(tx & mask) io_set_pin(SPISW_SDO_PORT, SPISW_SDO_PIN, 1);
        else io_set_pin(SPISW_SDO_PORT, SPISW_SDO_PIN, 0);
        
        /*SCK = 1*/
        io_set_pin(SPISW_SCK_PORT, SPISW_SCK_PIN, 1);
        
        /*Read SDI*/
        if(io_get_pin(SPISW_SDI_PORT, SPISW_SDI_PIN) != 0) rec++;
        
        mask = mask >> 1;
        
        /*SCK = 0*/
        io_set_pin(SPISW_SCK_PORT, SPISW_SCK_PIN, 0);
    }

    return rec;
}

#endif
