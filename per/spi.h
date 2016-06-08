/**
 * @file spi.h
 * 
 */

#ifndef SPI_H
#define SPI_H

/*********************
 *      INCLUDES
 *********************/
#include "../../hw_conf.h"
#if USE_SPI != 0

#include <stdint.h>
#include "hw/hw.h"
#include "psp/psp_spi.h"

/*********************
 *      DEFINES
 *********************/
#define SPI_CS_NUM  4
#define SPI_CS_SHIFT  2  
#define SPI_BAUD_MAX    0xFFFFFFFF

/**********************
 *      TYPEDEFS
 **********************/
typedef enum
{
    /*SPI1*/
    HW_SPI1_CS1 = 0,
    HW_SPI1_CS2,
    HW_SPI1_CS3,
    HW_SPI1_CS4,
    /*SPI2*/
    HW_SPI2_CS1,
    HW_SPI2_CS2,
    HW_SPI2_CS3,
    HW_SPI2_CS4,
    /*SPI3*/
    HW_SPI3_CS1,
    HW_SPI3_CS2,
    HW_SPI3_CS3,
    HW_SPI3_CS4,
    /*SPI4*/
    HW_SPI4_CS1,
    HW_SPI4_CS2,
    HW_SPI4_CS3,
    HW_SPI4_CS4,
    /*SPI5*/
    HW_SPI5_CS1,
    HW_SPI5_CS2,
    HW_SPI5_CS3,
    HW_SPI5_CS4,    
    /*SPISW*/
    HW_SPISW_CS1,
    HW_SPISW_CS2,
    HW_SPISW_CS3,
    HW_SPISW_CS4,
            
    HW_SPI_NUM 
}spi_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void spi_init(void);
void spi_cs_en(spi_t spi);
void spi_cs_dis(spi_t spi);
void spi_xchg(spi_t spi, const void * tx_buf, void * rx_buf, uint32_t length);
void spi_set_baud(spi_t spi, uint32_t baud);


/**********************
 *      MACROS
 **********************/

#endif

#endif
