/**
 * @file spi.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_SPI != 0 && PSP_PIC32MZ != 0

#include "../../spi.h"
#include <xc.h>

/*********************
 *      DEFINES
 *********************/
#define SPI_BAUD_DEF    1000000 /*Hz*/ 
#define SPI_BRG_MAX     2047 

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
/*SPIxCON                               SPIxSTAT                                 SPIxBRG,    SPIxBUF */
{NULL,									NULL,                                 	 NULL,		 NULL},     /*SPI0 not exists in PIC32MZ devices*/
#if defined(_SPI1CON_w_MASK) && SPI1_EN != 0
{(volatile __SPI2CONbits_t *) &SPI1CON, (volatile __SPI2STATbits_t *) &SPI1STAT, &SPI1BRG,   &SPI1BUF},
#else
{NULL,                                   NULL,                                   NULL,       NULL },
#endif

#if defined(_SPI2CON_w_MASK) && SPI2_EN != 0
{(volatile __SPI2CONbits_t *) &SPI2CON,  (volatile __SPI2STATbits_t *) &SPI2STAT, &SPI2BRG,  &SPI2BUF},
#else
{NULL,                                   NULL,                                   NULL,       NULL },
#endif

#if defined(_SPI3CON_w_MASK) && SPI3_EN != 0
{(volatile __SPI2CONbits_t *) &SPI3CON,  (volatile __SPI2STATbits_t *) &SPI3STAT, &SPI3BRG,  &SPI3BUF},
#else
{NULL,                                   NULL,                                   NULL,       NULL },
#endif

#if defined(_SPI4CON_w_MASK) && SPI4_EN != 0
{(volatile __SPI2CONbits_t *) &SPI4CON,  (volatile __SPI2STATbits_t *) &SPI4STAT, &SPI4BRG,  &SPI4BUF},
#else
{NULL,                                   NULL,                                   NULL,       NULL },
#endif

#if defined(_SPI5CON_w_MASK) && SPI5_EN != 0
{(volatile __SPI2CONbits_t *) &SPI5CON,  (volatile __SPI2STATbits_t *) &SPI5STAT, &SPI5BRG,  &SPI5BUF},
#else
{NULL,                                   NULL,                                   NULL,       NULL },
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

/**
 * Set a new baud rate for an SPI module
 * @param spi id of an SPI module from spi_hw_t enum
 * @param baud the new baud rate
 */
void psp_spi_set_baud(spi_hw_t spi, uint32_t baud)
{
    if(m_dsc[spi].SPIxBRG == NULL) return;
    
    if(baud == SPI_BAUD_MAX) {
        baud = CLOCK_PERIPH >> 1;
    }
    
    uint32_t brg;
    
    brg = (uint32_t) CLOCK_PERIPH / ((uint32_t) 2UL * baud);
    
    if(brg == 0) brg = 1; 
    
    brg --;
    
    if(brg > SPI_BRG_MAX) brg = SPI_BRG_MAX;
    
    *(m_dsc[spi].SPIxBRG) = brg;
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
    if(m_dsc[spi].SPIxBUF == NULL) return;
    
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
}

/**********************
 *   STATIC FUNCTIONS
 **********************/ 

#endif
