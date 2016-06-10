/**
 * @file spi.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_SPI != 0 && PSP_PIC24F_33F != 0

#include <stddef.h>
#include "hw/per/spi.h"
#include <xc.h>

/*********************
 *      DEFINES
 *********************/
#define SPI_BAUD_DEF    1000000 /*Hz*/ 
#define PPS_NUM (sizeof(pps_mcu) / sizeof(pps_mcu[0]))
#define SPS_NUM 8   /*Secondary prescale goes from 1 to 8*/


/**********************
 *      TYPEDEFS
 **********************/
typedef struct
{
    volatile SPI1CON1BITS * SPIxCON1;
    volatile SPI1CON2BITS * SPIxCON2;
    volatile SPI1STATBITS * SPIxSTAT;
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
/*          SPIxCON                               SPIxCON2,                         SPIxSTAT                     SPIxBUF */
#if defined (SPI2CON1) && SPI1_EN != 0
{(volatile SPI1CON1BITS *) &SPI1CON1, (volatile SPI1CON2BITS *) &SPI1CON2, (volatile SPI1STATBITS *) &SPI1STAT, &SPI1BUF},
#else
{NULL                               ,NULL,                                  NULL,                               NULL},
#endif
#if  defined(SPI2CON1) && SPI2_EN != 0
{(volatile SPI1CON1BITS *) &SPI2CON1, (volatile SPI1CON2BITS *) &SPI2CON2, (volatile SPI1STATBITS *) &SPI2STAT, &SPI2BUF},
#else
{NULL                               ,NULL,                                  NULL,                               NULL},
#endif
#if  defined(SPI3CON1) && SPI3_EN != 0
{(volatile SPI1CON1BITS *) &SPI3CON1, (volatile SPI1CON2BITS *) &SPI3CON2, (volatile SPI1STATBITS *) &SPI3STAT, &SPI3BUF},
#else
{NULL                               ,NULL,                                  NULL,                               NULL},
#endif
#if defined(SPI4CON1) && SPI4_EN != 0
{(volatile SPI1CON1BITS *) &SPI4CON1, (volatile SPI1CON2BITS *) &SPI4CON2, (volatile SPI1STATBITS *) &SPI4STAT, &SPI4BUF},
#else
{NULL                               ,NULL,                                  NULL,                               NULL},
#endif
#if defined(SPI5CON1) && SPI5_EN != 0
{(volatile SPI1CON1BITS *) &SPI5CON1, (volatile SPI1CON2BITS *) &SPI5CON2, (volatile SPI1STATBITS *) &SPI5STAT, &SPI5BUF}
#else
{NULL                               ,NULL,                                  NULL,                               NULL},
#endif
};

const static uint16_t pps_mcu[] = {64, 16, 4, 1};        /*Possible primary prescale values*/

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
    uint16_t * tmp_p;
    for(i = 0; i < SPI_HW_NUM; i++) {
        if(m_dsc[i].SPIxCON1 != NULL) { 
            tmp_p = (uint16_t *) m_dsc[i].SPIxCON1;
            *tmp_p = 0; 
            tmp_p = (uint16_t *) m_dsc[i].SPIxCON2;
            *tmp_p = 0; 
            tmp_p = (uint16_t *) m_dsc[i].SPIxSTAT;
            *tmp_p = 0; 
            m_dsc[i].SPIxCON1->MODE16 = 0;  
            m_dsc[i].SPIxCON1->MSTEN = 1;     
            m_dsc[i].SPIxCON1->CKP = 1;
            m_dsc[i].SPIxCON1->SMP = 1;       
            m_dsc[i].SPIxSTAT->SPIROV = 0;
            psp_spi_set_baud(i, SPI_BAUD_DEF);

            
            m_dsc[i].SPIxSTAT->SPIEN = 1;
        }
    }
}

/**
 * Set a new baud rate for an SPI module
 * @param spi id of the spi module (from spi_t enum)
 * @param baud the new baud rate (SPI_BAUD_MAX for max possible baud rate)
 */
void psp_spi_set_baud(spi_hw_t spi, uint32_t baud)
{
    if(baud == SPI_BAUD_MAX) baud = CLOCK_PERIPH / 2;
    
    uint32_t div = (uint32_t)CLOCK_PERIPH / baud;
    
    if(div <= 1) div = 2;
    
    uint8_t sps = SPS_NUM;    /*Calculated Secondary prescale*/
    int8_t pps;          /*Calculated Primary prescale*/
    uint32_t div_tmp = 0;
    /*Calculate the Primary prescale to get a div value between SPS_MIN and SPS_MAX*/
    for(pps = PPS_NUM - 1; pps >= 0; pps --) {
        div_tmp = div / pps_mcu[pps];
        if(div_tmp <= SPS_NUM) {
            /*The remaining div will be the Secondary prescale*/
            sps = SPS_NUM - div_tmp;
            break;
        }
    }
    
    m_dsc[spi].SPIxSTAT->SPIEN = 0;
    m_dsc[spi].SPIxCON1->PPRE = pps;
    m_dsc[spi].SPIxCON1->SPRE = sps;
    m_dsc[spi].SPIxSTAT->SPIEN = 1;
    
}

/**
 * Make a transfer on SPI
 * @param spi id of the spi module (from spi_t enum)
 * @param tx_a pointer to variable with the bytes to send (if NULL 0xFF will be sent)
 * @param rx_a pointer to variable where to store the received data (can be NULL)
 * @param length number of bytes to send/receive
 */
void psp_spi_xchg(spi_hw_t spi, const void * tx_a, void * rx_a, uint32_t length)
{   
    if(m_dsc[spi].SPIxCON1 == NULL) return;
    
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
