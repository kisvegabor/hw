/**
 * @file psp_spi.h
 * 
 */

#ifndef PSP_SPI_H
#define PSP_SPI_H

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum
{
    SPI_HW0,
    SPI_HW1,
    SPI_HW2,
    SPI_HW3,
    SPI_HW4,
    SPI_HW5,
    SPI_HW_NUM,
    SPI_HW_INV = 0xFF,
}spi_hw_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void psp_spi_init(void);
void psp_spi_set_baud(spi_hw_t spi, uint32_t baud);
void psp_spi_xchg(spi_hw_t spi, const void * tx_a, void * rx_a, uint32_t length);

/**********************
 *      MACROS
 **********************/

#endif
