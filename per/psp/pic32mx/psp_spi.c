/**
 * @file spi.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_SPI != 0 && PSP_PIC32MX != 0

#include "../../spi.h"
#include <xc.h>

/*********************
 *      DEFINES
 *********************/
#define SPI_BAUD_DEF    1000000 /*Hz*/ 

/**********************
 *      TYPEDEFS
 **********************/
typedef struct
{
    volatile __SPI2CONbits_t * SPIxCON;
    volatile __SPI2STATbits_t * SPIxSTAT;
    volatile unsigned int * SPIxBRG;
    volatile unsigned int * SPIxBUF;
}m_dsc_t;


/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static m_dsc_t m_dsc[] = 
{
/*SPIxCON                               SPIxSTAT                                   SPIxBRG,    SPIxBUF */
{(volatile __SPI2CONbits_t *) &SPI1ACON, (volatile __SPI2STATbits_t *) &SPI1ASTAT, &SPI1ABRG, &SPI1ABUF},
{(volatile __SPI2CONbits_t *) &SPI2CON,  (volatile __SPI2STATbits_t *) &SPI2STAT, &SPI2BRG,   &SPI2BUF},
{(volatile __SPI2CONbits_t *) &SPI3CON,  (volatile __SPI2STATbits_t *) &SPI3STAT, &SPI3BRG,   &SPI3BUF},
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void psp_spi_init(void)
{
    spi_hw_t i;
    for(i = 0; i < SPI_HW_NUM; i++) {
        if(m_dsc[i].SPIxCON != NULL) {
            m_dsc[i].SPIxCON->CKP = 1;
            m_dsc[i].SPIxCON->SMP = 1;        
            m_dsc[i].SPIxCON->MODE16 = 0;
            m_dsc[i].SPIxCON->MODE32 = 0;
            m_dsc[i].SPIxCON->MSTEN = 1;       
            m_dsc[i].SPIxSTAT->SPIROV = 0;

            psp_spi_set_baud(i, SPI_BAUD_DEF);
            
            m_dsc[i].SPIxCON->ON = 1;
        }
    }
}

hw_res_t psp_spi_set_baud(spi_hw_t spi, uint32_t baud)
{
    if(baud == SPI_BAUD_MAX) {
        baud = CLOCK_PERIPH / 2;
    }
    
    uint32_t brg;
    
    brg = (uint32_t) CLOCK_PERIPH / ((uint32_t) 2UL * baud);
    
    if(brg == 0) return HW_RES_INV_PARAM;
    
    brg --;
    
    *(m_dsc[spi].SPIxBRG) = brg;
    
    return HW_RES_OK;
}

hw_res_t psp_spi_xchg(spi_hw_t spi, const void * tx_a, void * rx_a, uint32_t length)
{   
    const uint8_t * tx8_a = tx_a;
    uint8_t * rx8_a = rx_a;
    uint8_t rec;
    uint8_t send = 0xFF;
    uint32_t i = 0;
    
    for(i = 0; i < length; i++ ) {
        if(tx8_a != NULL) send = tx8_a[i];
    
        *(m_dsc[spi].SPIxBUF) = send;
        while(!(m_dsc[spi].SPIxSTAT->SPIRBF));
        rec = (uint8_t)*(m_dsc[spi].SPIxBUF);
        
        if(rx8_a != NULL) rx8_a[i] = rec;
    }
    
	return HW_RES_OK;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/ 

#endif
