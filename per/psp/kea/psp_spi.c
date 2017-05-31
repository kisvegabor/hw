/**
 * @file spi.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_SPI != 0 && PSP_KEA != 0

#include "derivative.h" /* include peripheral declarations SSKEAZ128M4 */
#include "../../spi.h"
#include <stddef.h>

/*********************
 *      DEFINES
 *********************/
#define SPI_BAUD_DEF    1000000 /*Hz*/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct
{
    volatile SPI_Type * dsc;
}m_dsc_t;


/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static m_dsc_t m_dsc[] = 
{
/*dsc*/
#if defined(SPI0_BASE) && SPI0_EN != 0
{SPI0},
#else
{NULL},
#endif

#if defined(SPI1_BASE) && SPI1_EN != 0
{SPI1},
#else
{NULL},
#endif

#if defined(SPI2_BASE) && SPI2_EN != 0
{SPI2},
#else
{NULL},
#endif

#if defined(SPI3_BASE) && SPI3_EN != 0
{SPI3},
#else
{NULL},
#endif

#if defined(SPI4_BASE) && SPI4_EN != 0
{SPI4},
#else
{NULL},
#endif

#if defined(SPI5_BASE) && SPI5_EN != 0
{SPI5},
#else
{NULL},
#endif

};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the SPI modules
 */
void psp_spi_init(void)
{
    spi_hw_t i;
    for(i = 0; i < SPI_HW_NUM; i++) {
        if(m_dsc[i].dsc != NULL) {
        	m_dsc[i].dsc->C1 |= SPI_C1_MSTR_MASK; 			// SPI1 master mode
        	m_dsc[i].dsc->C1 &= (~SPI_C1_SSOE_MASK);
			m_dsc[i].dsc->C1 |= SPI_C1_CPHA_MASK;  			// clock polarity
			m_dsc[i].dsc->C1 |= SPI_C1_CPOL_MASK;  			//clock phase
			m_dsc[i].dsc->C1 &= (~SPI_C1_LSBFE_MASK);  		// LSB:most significant
			m_dsc[i].dsc->C1 &= (~SPI_C1_SPIE_MASK);        //Disable RX interrrupt
			m_dsc[i].dsc->C1 &= (~SPI_C1_SPTIE_MASK);       //Disable the transmit interrupt
        	m_dsc[i].dsc->C2 &= (~SPI_C2_MODFEN_MASK);

            psp_spi_set_baud(i, SPI_BAUD_DEF);
            SPI1_BR = 0x43;  			//SPPR = 4, SPR = 3, bps div = (SPPR+1)*2^(SPR+1) = 80, baudrate= 24Mhz/80=300khz

        	m_dsc[i].dsc->C1 |= SPI_C1_SPE_MASK;  // enable SPI module
        }
    }
}

/**
 * Set a new baud rate for an SPI module
 * @param spi id of an SPI module from spi_hw_t enum
 * @param baud the new baud rate
 */
void psp_spi_set_baud(spi_hw_t spi, uint32_t baud)
{
//    if(m_dsc[spi].SPIxBRG == NULL) return;
//
//    if(baud == SPI_BAUD_MAX) {
//        baud = CLOCK_PERIPH >> 1;
//    }
//
//    uint32_t brg;
//
//    brg = (uint32_t) CLOCK_PERIPH / ((uint32_t) 2UL * baud);
//
//    if(brg == 0) brg = 1;
//
//    brg --;
//
//    if(brg > SPI_BRG_MAX) brg = SPI_BRG_MAX;
//
//    *(m_dsc[spi].SPIxBRG) = brg;
}

/**
 * Make an SPI transfer
 * @param spi id of an SPI module from spi_hw_t enum
 * @param tx_a pointer to array to send (if NULL 0xFF will be sent)
 * @param rx_a pointer to buffer to store the received bytes (NULL if ignored)
 * @param length
 */
void psp_spi_xchg(spi_hw_t spi, const void * tx_a, void * rx_a, uint32_t length)
{   
    if(m_dsc[spi].dsc == NULL) return;
    
    const uint8_t * tx8_a = tx_a;
    uint8_t * rx8_a = rx_a;
    uint8_t rec;
    uint8_t send = 0xFF;
    uint32_t i = 0;
    
    for(i = 0; i < length; i++ ) {
        if(tx8_a != NULL) send = tx8_a[i];
        while((m_dsc[spi].dsc->S & SPI_S_SPTEF_MASK) == 0);	/*Wait for empty transmit buffer*/
        m_dsc[spi].dsc->D = send;
        while ((m_dsc[spi].dsc->S & SPI_S_SPRF_MASK) == 0); /* wait ready buffer full */
        rec = m_dsc[spi].dsc->D;
        if(rx8_a != NULL) rx8_a[i] = rec;
    }

}

/**********************
 *   STATIC FUNCTIONS
 **********************/ 

#endif
