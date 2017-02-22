/**
@file psp_serial.c 
 */

/***********************
 *       INCLUDES
 ***********************/
#include "hw_conf.h"
#if USE_SERIAL != 0 && PSP_PIC24F_33F != 0

#include <xc.h>
#include <stddef.h>
#include "hw/hw.h"
#include "misc/mem/fifo.h"
#include "hw/per/tick.h"
#include "../psp_serial.h"

/***********************
 *       DEFINES
 ***********************/
#ifndef SERIAL_DEF_BAUD
#define SERIAL_DEF_BAUD 9600
#endif

#define SERIAL1_RX_IF IFS0bits.U1RXIF
#define SERIAL1_TX_IF IFS0bits.U1TXIF
#define SERIAL1_RX_IE IEC0bits.U1RXIE
#define SERIAL1_TX_IE IEC0bits.U1TXIE
#define SERIAL1_RX_IP IPC2bits.U1RXIP
#define SERIAL1_TX_IP IPC3bits.U1TXIP

#define SERIAL2_RX_IF IFS1bits.U2RXIF
#define SERIAL2_TX_IF IFS1bits.U2TXIF
#define SERIAL2_RX_IE IEC1bits.U2RXIE
#define SERIAL2_TX_IE IEC1bits.U2TXIE
#define SERIAL2_RX_IP IPC7bits.U2RXIP
#define SERIAL2_TX_IP IPC7bits.U2TXIP

#define SERIAL3_RX_IF IFS5bits.U3RXIF
#define SERIAL3_TX_IF IFS5bits.U3TXIF
#define SERIAL3_RX_IE IEC5bits.U3RXIE
#define SERIAL3_TX_IE IEC5bits.U3TXIE
#define SERIAL3_RX_IP IPC20bits.U3RXIP
#define SERIAL3_TX_IP IPC20bits.U3TXIP

#define SERIAL4_RX_IF IFS5bits.U4RXIF
#define SERIAL4_TX_IF IFS5bits.U4TXIF
#define SERIAL4_RX_IE IEC5bits.U4RXIE
#define SERIAL4_TX_IE IEC5bits.U4TXIE
#define SERIAL4_RX_IP IPC22bits.U4RXIP
#define SERIAL4_TX_IP IPC22bits.U4TXIP

/* Macro to check an SERIAL if enabled or not in the configurations (drv_conf)
 * x: modul ID
 * Usage: #if SERIAL_MODULE_EN(2) ... #endif */
#define SERIAL_MODULE_EN(x) (SERIAL ## x ##_BUF_SIZE != 0 && SERIAL ## x ##_PRIO != HW_INT_PRIO_OFF)

/***********************
 *       TYPEDEFS
 ***********************/
typedef union {
    struct {
      unsigned URXDA:1;
      unsigned OERR:1;
      unsigned FERR:1;
      unsigned PERR:1;
      unsigned RIDLE:1;
      unsigned ADDEN:1;
      unsigned URXISEL:2;
      unsigned TRMT:1;
      unsigned UTXBF:1;
      unsigned UTXEN:1;
      unsigned UTXBRK:1;
      unsigned URXEN:1;
      unsigned UTXISEL0:1;
      unsigned UTXINV:1;
      unsigned UTXISEL1:1;
    };
}MY_U1STABITS;
    
typedef struct
{
    U1MODEBITS * UxMODE; 
    MY_U1STABITS * UxSTA; 
    volatile unsigned int * UxBRG;
    volatile unsigned int * tx_reg;
    volatile unsigned int * rx_reg;
    uint32_t buf_size;
    uint8_t mode;
    fifo_t tx_fifo;
    fifo_t rx_fifo;
}m_dsc_t;

/***********************
 *   GLOBAL VARIABLES
 ***********************/

/***********************
 *   STATIC VARIABLES
 ***********************/

/*Create fifos for every SERIAL module*/
#if SERIAL_MODULE_EN(1)
static uint8_t tbuf1[SERIAL1_BUF_SIZE];
static uint8_t rbuf1[SERIAL1_BUF_SIZE];
#endif
#if SERIAL_MODULE_EN(2)
static uint8_t tbuf2[SERIAL2_BUF_SIZE];
static uint8_t rbuf2[SERIAL2_BUF_SIZE];
#endif
#if SERIAL_MODULE_EN(3)
static uint8_t tbuf3[SERIAL3_BUF_SIZE];
static uint8_t rbuf3[SERIAL3_BUF_SIZE];
#endif
#if SERIAL_MODULE_EN(4)
static uint8_t tbuf4[SERIAL4_BUF_SIZE];
static uint8_t rbuf4[SERIAL4_BUF_SIZE];
#endif


static m_dsc_t m_dsc[] = 
{
    /*UxMODE                 UxSTA                 UxBRG   UxTXREG    U1RXREG   buf_size               mode */
#if SERIAL_MODULE_EN(1)
    {(U1MODEBITS *) &U1MODE, (MY_U1STABITS *) &U1STA, &U1BRG, &U1TXREG,  &U1RXREG, SERIAL1_BUF_SIZE, SERIAL1_MODE},
#else
    {NULL,                 NULL,                   NULL,   NULL,      NULL,     0,                       0},
#endif          
#if SERIAL_MODULE_EN(2)
    {(U1MODEBITS *) &U2MODE, (MY_U1STABITS *) &U2STA, &U2BRG, &U2TXREG,  &U2RXREG, SERIAL2_BUF_SIZE, SERIAL2_MODE},
#else
    {NULL,                 NULL,                   NULL,   NULL,      NULL,     0,                       0},
#endif        
#if SERIAL_MODULE_EN(3)
    {(U1MODEBITS *) &U3MODE, (MY_U1STABITS *) &U3STA, &U3BRG, &U3TXREG,  &U3RXREG, SERIAL3_BUF_SIZE, SERIAL3_MODE},
#else
    {NULL,                 NULL,                   NULL,   NULL,      NULL,     0,                       0},
#endif        
#if SERIAL_MODULE_EN(4)
    {(U1MODEBITS *) &U4MODE, (MY_U1STABITS *) &U4STA, &U4BRG, &U4TXREG,  &U4RXREG, SERIAL4_BUF_SIZE, SERIAL4_MODE},},
#else
    {NULL,                 NULL,                   NULL,   NULL,      NULL,     0,                       0},
#endif
};


/***********************
 *   GLOBAL PROTOTYPES
 ***********************/

/***********************
 *   STATIC PROTOTYPES
 ***********************/
static void psp_serial_init_irq(void);
static void psp_serial_init_module(void);
static void psp_serial_send_next(serial_t id);
static void psp_serial_rec_next(serial_t modul_id);
static void psp_serial_rx_int_en(serial_t id, uint8_t state);
static void psp_serial_tx_int_en(serial_t id, uint8_t state);

/***********************
 *   GLOBAL FUNCTIONS
 ***********************/

/**
 * Initialize the UART modules
 */
void psp_serial_init(void)
{
    psp_serial_init_irq();  /*Interrupt init*/
    
    psp_serial_init_module(); /*serial register init*/
}

/**
 * Send a byte via UART
 * @param id the id of the UART module (from serial_t enum)
 * @param tx byte to send
 * @return HW_RES_OK or any error from hw_res_t
 */
hw_res_t psp_serial_wr(serial_t id, uint8_t tx)
{
    
    hw_res_t res = HW_RES_OK;
    
    /*If a register is NULL then the module is disabled*/
    if(m_dsc[id].UxMODE != NULL) {
        bool fifo_ret;
        /*The fifo is used in the interrupt so disable interrupts*/
        psp_serial_tx_int_en(id, 0); 
        fifo_ret = fifo_push(&m_dsc[id].tx_fifo, &tx); 
        /* If data is added to the fifo start sending*/
        if(fifo_ret != false && m_dsc[id].UxSTA->TRMT != 0) {
            psp_serial_send_next(id);
        }
        
        psp_serial_tx_int_en(id, 1);
        
        /*Show the fifo become full so not all bytes are buffered*/
        if(fifo_ret == false) {
            res = HW_RES_FULL;
            /*Show different error if the CPU priority is higher 
             * then the TX interrupt priority. 
             * In this case the sending is not possible*/
          //  if(SRbits.IPL > SERIAL1_PRIO) res = HW_RES_NOT_RDY;
        }
    } else {
        res = HW_RES_DIS;
    }
    
    return res;
}

/**
 * Receive a byte from UART
 * @param id the id of the UART module (from serial_t enum)
 * @param rx pointer to variable to store the received byte
 * @return HW_RES_OK or any error from hw_res_t
 */
hw_res_t psp_serial_rd(serial_t id, uint8_t * rx)
{
    hw_res_t res = HW_RES_OK;
    
    /*If a register is NULL then the module is disabled*/
    if(m_dsc[id].UxMODE == NULL) return HW_RES_DIS;
    
    bool fifo_ret;
    /*The fifo is used in the interrupt so disable interrupts*/
    psp_serial_rx_int_en(id, 0); 
    fifo_ret = fifo_pop(&m_dsc[id].rx_fifo, rx); 
    psp_serial_rx_int_en(id, 1); 

    if(fifo_ret == false)  return HW_RES_EMPTY;
    
    
    return res;
}

/**
 * Set the baud rate of the UART module
 * @param id the id of the UART module (from serial_t enum)
 * @param baud the new baud rate
 * @return HW_RES_OK or any error from hw_res_t
 */
hw_res_t psp_serial_set_baud(serial_t id, uint32_t baud)
{
    hw_res_t res = HW_RES_OK;
    
    /*If a register is NULL then the module is disabled*/
    if(m_dsc[id].UxMODE != NULL) {
        uint32_t brg = ((uint32_t) CLOCK_PERIPH / baud / 4) - 1;
        m_dsc[id].UxMODE->BRGH = 1;
        *m_dsc[id].UxBRG = brg;
    } else {
        res = HW_RES_DIS;
    }
    
    return res;
}

/**
 * Clear all data from the rx buffer
 * @param id the id of the UART module (from serial_t enum)
 * @return HW_RES_OK or any error from hw_res_t
 */
hw_res_t psp_serial_clear_rx_buf(serial_t id)
{
    hw_res_t res = HW_RES_OK;
    if(m_dsc[id].UxMODE != NULL) {
        fifo_clear(&m_dsc[id].rx_fifo);
    } else {
        res = HW_RES_DIS;
    }
    
    return res;
}

#if SERIAL_MODULE_EN(1)
/**
 * Called when a byte is send on the  SERIAL1 modul
 */
void __attribute__((__interrupt__, auto_psv )) _ISR _U1TXInterrupt (void)
{
    SERIAL1_TX_IF = 0;
    
    psp_serial_send_next(HW_SERIAL1);
}

/**
 * Called when a byte is received on  SERIAL1 modul
 */
void __attribute__((__interrupt__, auto_psv )) _ISR _U1RXInterrupt (void)
{
    SERIAL1_RX_IF = 0;
    while(U1STAbits.URXDA)  psp_serial_rec_next(HW_SERIAL1);
}
#endif

#if SERIAL_MODULE_EN(2)
/**
 * Called when a byte is send on the  SERIAL2 modul
 */
void __attribute__((__interrupt__, auto_psv )) _ISR _U2TXInterrupt (void)
{
    SERIAL2_TX_IF = 0;

    psp_serial_send_next(HW_SERIAL2);
}


/**
 * Called when a byte is received on  SERIAL2 modul
 */
void __attribute__((__interrupt__, auto_psv )) _ISR _U2RXInterrupt (void)
{
    SERIAL2_RX_IF = 0;
    while(U2STAbits.URXDA) psp_serial_rec_next(HW_SERIAL2);
}
#endif

#if SERIAL_MODULE_EN(3)
/**
 * Called when a byte is send on the SERIAL3 modul
 */
void __attribute__((__interrupt__, auto_psv )) _ISR _U3TXInterrupt (void)
{
    SERIAL3_TX_IF = 0;

    psp_serial_send_next(HW_SERIAL3);
}

/**
 * Called when a byte is received on SERIAL3 modul
 */
void __attribute__((__interrupt__, auto_psv )) _ISR _U3RXInterrupt (void)
{
    SERIAL3_RX_IF = 0;
    while(U3STAbits.URXDA) psp_serial_rec_next(HW_SERIAL3);
}
#endif

#if SERIAL_MODULE_EN(4)
/**
 * Called when a byte is send on the SERIAL4 modul
 */
void __attribute__((__interrupt__, auto_psv )) _ISR _U4TXInterrupt (void)
{
    SERIAL4_TX_IF = 0;

    psp_serial_send_next(HW_SERIAL4);
}

/**
 * Called when a byte is received on SERIAL4 modul
 */
void __attribute__((__interrupt__, auto_psv )) _ISR _U4RXInterrupt (void)
{
    SERIAL4_RX_IF = 0;
    while(U4STAbits.URXDA)  psp_serial_rec_next(HW_SERIAL4);
}
#endif

/***********************
 *   STATIC FUNCTIONS
 ***********************/

/**
 * Initialize the interrupts
 */
static void psp_serial_init_irq(void)
{
#ifdef SERIAL1_RX_IF
#if SERIAL_MODULE_EN(1)
    
    fifo_init(&m_dsc[HW_SERIAL1].tx_fifo, tbuf1, sizeof(uint8_t), SERIAL1_BUF_SIZE);
    fifo_init(&m_dsc[HW_SERIAL1].rx_fifo, rbuf1, sizeof(uint8_t), SERIAL1_BUF_SIZE);
    
    SERIAL1_RX_IF = 0;
    SERIAL1_TX_IF = 0;
    SERIAL1_RX_IE = 1;
    SERIAL1_TX_IE = 1;
    SERIAL1_RX_IP = SERIAL1_PRIO;
    SERIAL1_TX_IP = SERIAL1_PRIO;
#endif
#endif

#ifdef SERIAL2_RX_IF
#if SERIAL_MODULE_EN(2)
    fifo_init(&m_dsc[HW_SERIAL2].tx_fifo, tbuf2, sizeof(uint8_t), SERIAL2_BUF_SIZE);
    fifo_init(&m_dsc[HW_SERIAL2].rx_fifo, rbuf2, sizeof(uint8_t), SERIAL2_BUF_SIZE);
    
    SERIAL2_RX_IF = 0;
    SERIAL2_TX_IF = 0;
    SERIAL2_RX_IE = 1;
    SERIAL2_TX_IE = 1;
    SERIAL2_RX_IP = SERIAL2_PRIO;
    SERIAL2_TX_IP = SERIAL2_PRIO;
#endif
#endif

#ifdef SERIAL3_RX_IF
#if SERIAL_MODULE_EN(3)
    fifo_init(&m_dsc[HW_SERIAL3].tx_fifo, tbuf3, sizeof(uint8_t), SERIAL3_BUF_SIZE);
    fifo_init(&m_dsc[HW_SERIAL3].rx_fifo, rbuf3, sizeof(uint8_t), SERIAL3_BUF_SIZE);
    
    SERIAL3_RX_IF = 0;
    SERIAL3_TX_IF = 0;
    SERIAL3_RX_IE = 1;
    SERIAL3_TX_IE = 1;
    SERIAL3_RX_IP = SERIAL3_PRIO;
    SERIAL3_TX_IP = SERIAL3_PRIO;
    
#endif
#endif

#ifdef SERIAL4_RX_IF
#if SERIAL_MODULE_EN(4)
    
    fifo_init(&m_dsc[HW_SERIAL4].tx_fifo, tbuf4, sizeof(uint8_t), SERIAL4_BUF_SIZE);
    fifo_init(&m_dsc[HW_SERIAL4].rx_fifo, rbuf4, sizeof(uint8_t), SERIAL4_BUF_SIZE);
    
    SERIAL4_RX_IF = 0;
    SERIAL4_TX_IF = 0;
    SERIAL4_RX_IE = 1;
    SERIAL4_TX_IE = 1;
    SERIAL4_RX_IP = SERIAL4_PRIO;
    SERIAL4_TX_IP = SERIAL4_PRIO;
#endif
#endif
    
}

/**
 * Init the modules
 */
static void psp_serial_init_module(void)
{
    /*Init all modules*/
    serial_t id;    
    for(id = HW_SERIAL1; id < HW_SERIAL_NUM; id++) {
        /*Test ID. Only the valid (inited and used) are inited forward */
        if(m_dsc[id].UxMODE != NULL) {         
            /*Init the modules*/
            psp_serial_set_baud(id, SERIAL_DEF_BAUD);

            m_dsc[id].UxMODE->UEN = 0b00; /*00 = SERIAL: UxTX, UxRX, I/O: UxCTS, UxRTS */
            m_dsc[id].UxMODE->UARTEN = 1;    /*SERIALx is enable*/
            m_dsc[id].UxSTA->UTXISEL0 = 1;
            m_dsc[id].UxSTA->UTXISEL1 = 0;
            m_dsc[id].UxSTA->UTXEN = 1;   /*SERIALx TX int. enable*/
            m_dsc[id].UxSTA->URXEN = 1;   /*SERIALx RX int. enable*/
        }
    }   
    
}

/**
 * Send the next byte from tx FIFO
 * @param id the id of the UART module (from serial_t enum)
 */
static void psp_serial_send_next(serial_t id)
{
    
    //pop the data from the buffer and send it
    uint8_t tx_byte;
    if(fifo_pop(&m_dsc[id].tx_fifo, &tx_byte) != false) {
        *m_dsc[id].tx_reg = tx_byte;
    }
}

/**
 * Read the UART module and push the read byte into the rx FIFO
 * @param id the id of the UART module (from serial_t enum)
 */
static void psp_serial_rec_next(serial_t id)
{
    m_dsc_t * dsc = &m_dsc[id];
    
    uint8_t rec_data = *(dsc->rx_reg);
    
    if(m_dsc[id].UxSTA->OERR != 0) m_dsc[id].UxSTA->OERR = 0;
    
    //There is space in the buffer (not full)
    if(fifo_get_free(&dsc->rx_fifo) != 0){
        fifo_push(&dsc->rx_fifo, &rec_data);
    }
}

/**
 * Enable or disable the rx interrupts
 * @param id the id of the UART module (from serial_t enum)
 * @param state 0: disable interrupt, 1: enable interrupt
 */
static void psp_serial_rx_int_en(serial_t id, uint8_t state)
{
    //State can be only 0 or 1
    if(state) state = 1;

    switch (id)
    {
#if SERIAL_MODULE_EN(1)
        case HW_SERIAL1:
            SERIAL1_RX_IE = state;
            break;
#endif

#if SERIAL_MODULE_EN(2)
        case HW_SERIAL2:
            SERIAL2_RX_IE = state;
            break;
#endif

#if SERIAL_MODULE_EN(3)
        case HW_SERIAL3:
            SERIAL3_RX_IE = state;
            break;
#endif

#if SERIAL_MODULE_EN(4)
        case HW_SERIAL4:
            SERIAL4_RX_IE = state;
            break;
#endif
        default:
            break;
    }
}

/**
 * Enable or disable the tx interrupts
 * @param id the id of the UART module (from serial_t enum)
 * @param state 0: disable interrupt, 1: enable interrupt
 */
static void psp_serial_tx_int_en(serial_t id, uint8_t state)
{
   
    //State can be only 0 or 1
    if(state)  state = 1;

    switch (id)
    {
#if SERIAL_MODULE_EN(1)
        case HW_SERIAL1:
            SERIAL1_TX_IE = state;
            break;
#endif

#if SERIAL_MODULE_EN(2)
        case HW_SERIAL2:
            SERIAL2_TX_IE = state;
            break;
#endif

#if SERIAL_MODULE_EN(3)
        case HW_SERIAL3:
            SERIAL3_TX_IE = state;
            break;
#endif

#if SERIAL_MODULE_EN(4)
        case HW_SERIAL4:
            SERIAL4_TX_IE = state;
            break;
#endif

        default:
            break;
    }
}

#endif
