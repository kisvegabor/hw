/**
@file psp_serial.c 
 */

/***********************
 *       INCLUDES
 ***********************/
#include "hw_conf.h"
#if USE_SERIAL != 0 && PSP_KEA != 0

#include "derivative.h"
#include <stddef.h>
#include "hw/hw.h"
#include "misc/mem/fifo.h"
#include "../psp_serial.h"

/***********************
 *       DEFINES
 ***********************/
#define SERIAL_DEF_BAUD 9600

/* Macro to check an SERIAL if enabled or not in the configurations (drv_conf)
 * x: modul ID
 * Usage: #if SERIAL_MODULE_EN(2) ... #endif */
#define SERIAL_MODULE_EN(x) (SERIAL ## x ##_BUF_SIZE != 0 && SERIAL ## x ##_PRIO != HW_INT_PRIO_OFF)


/***********************
 *       TYPEDEFS
 ***********************/
   
typedef struct
{
	UART_Type * dsc;
	uint32_t buf_size;
	uint32_t clk_en_mask;
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
#if SERIAL_MODULE_EN(0)
static uint8_t tbuf0[SERIAL1_BUF_SIZE];
static uint8_t rbuf0[SERIAL1_BUF_SIZE];
#endif
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


static m_dsc_t m_dsc[HW_SERIAL_NUM] =
{
    /*dsc         	  buf_size               mode */
#if SERIAL_MODULE_EN(0)
    {UART0, 		SERIAL0_BUF_SIZE, 		SERIAL0_MODE},
#else
    {NULL,               0,                       0},
#endif
#if SERIAL_MODULE_EN(1)
    {UART1, 		SERIAL1_BUF_SIZE, 		SERIAL1_MODE},
#else
    {NULL,               0,                       0},
#endif
#if SERIAL_MODULE_EN(2)
    {UART2, 		SERIAL2_BUF_SIZE, 		SERIAL2_MODE},
#else
    {NULL,               0,                       0},
#endif
#if SERIAL_MODULE_EN(3)
    {UART3, 		SERIAL3_BUF_SIZE, 		SERIAL3_MODE},
#else
    {NULL,               0,                       0},
#endif
};


/***********************
 *   GLOBAL PROTOTYPES
 ***********************/

/***********************
 *   STATIC PROTOTYPES
 ***********************/
static void irq_generic(serial_t id);
static void psp_serial_rx_int_en(serial_t id, bool en);
static void psp_serial_tx_int_en(serial_t id, bool en);

/***********************
 *   GLOBAL FUNCTIONS
 ***********************/

/**
 * Initialize the UART modules
 */
void psp_serial_init(void)
{

	uint8_t i;

	for(i = 0; i < HW_SERIAL_NUM; i++) {
		if(m_dsc[i].dsc == NULL) continue;

		m_dsc[i].dsc->BDH = 0;			 /* One stop bit; upper baud divisor bits = 0 */
		psp_serial_set_baud(i, SERIAL_DEF_BAUD);
		m_dsc[i].dsc->C1  = 0;                     /* Initialize control bits for communication: */


		/* M (9 or 8 bit select) = 0 (default, 8 bit format) */
		/* PE (Parity Enable) = 0 (default, no parity) */
		/* UARTSWAI (UART stops in wait mode)=0 (default, no stop)*/
		/* WAKE (Recvr Wakeup Method) = 0 */
		/*      (default, idle-line wakeup) */
		m_dsc[i].dsc->C2 = 0x0C;                  /* Enable Tx, Rx. No IRQs, Rx in standby, break char */

		if(i == 0) {
#if SERIAL_MODULE_EN(0)

			NVIC_ClearPendingIRQ(UART0_IRQn);  /* Clear any Pending IRQ for all PIT ch0 (#22) */
			NVIC_EnableIRQ(UART0_IRQn);        /* Set Enable IRQ for PIT_CH0 */
			NVIC_SetPriority(UART0_IRQn,0);    /* Set Priority for PIT_CH0 */
			fifo_init(m_dsc[i].rx_fifo, rbuf0, sizeof(uint8_t), sizeof(rbuf0));
			fifo_init(m_dsc[i].tx_fifo, tbuf0, sizeof(uint8_t), sizeof(tbuf0));
#endif
		} else if(i == 1) {
#if SERIAL_MODULE_EN(1)

			NVIC_ClearPendingIRQ(UART1_IRQn);  /* Clear any Pending IRQ for all PIT ch0 (#22) */
			NVIC_EnableIRQ(UART1_IRQn);        /* Set Enable IRQ for PIT_CH0 */
			NVIC_SetPriority(UART1_IRQn,0);    /* Set Priority for PIT_CH0 */
			fifo_init(&m_dsc[i].rx_fifo, rbuf1, sizeof(uint8_t), sizeof(rbuf1));
			fifo_init(&m_dsc[i].tx_fifo, tbuf1, sizeof(uint8_t), sizeof(tbuf1));
#endif
		} else if(i == 2) {
#if SERIAL_MODULE_EN(2)
			NVIC_ClearPendingIRQ(UART2_IRQn);  /* Clear any Pending IRQ for all PIT ch0 (#22) */
			NVIC_EnableIRQ(UART2_IRQn);        /* Set Enable IRQ for PIT_CH0 */
			NVIC_SetPriority(UART2_IRQn,0);    /* Set Priority for PIT_CH0 */
			fifo_init(&m_dsc[i].rx_fifo, rbuf2, sizeof(uint8_t), sizeof(rbuf2));
			fifo_init(&m_dsc[i].tx_fifo, tbuf2, sizeof(uint8_t), sizeof(tbuf2));
#endif
		} else if(i == 3) {
#if SERIAL_MODULE_EN(3)
			NVIC_ClearPendingIRQ(UART3_IRQn);  /* Clear any Pending IRQ for all PIT ch0 (#22) */
			NVIC_EnableIRQ(UART3_IRQn);        /* Set Enable IRQ for PIT_CH0 */
			NVIC_SetPriority(UART3_IRQn,0);    /* Set Priority for PIT_CH0 */
			fifo_init(m_dsc[i].rx_fifo, rbuf3, sizeof(uint8_t), sizeof(rbuf3));
			fifo_init(m_dsc[i].tx_fifo, tbuf3, sizeof(uint8_t), sizeof(tbuf3));
#endif
		} else if(i == 4) {
#if SERIAL_MODULE_EN(4)
			NVIC_ClearPendingIRQ(UART4_IRQn);  /* Clear any Pending IRQ for all PIT ch0 (#22) */
			NVIC_EnableIRQ(UART4_IRQn);        /* Set Enable IRQ for PIT_CH0 */
			NVIC_SetPriority(UART4_IRQn,0);    /* Set Priority for PIT_CH0 */
			fifo_init(m_dsc[i].rx_fifo, rbuf4, sizeof(uint8_t), sizeof(rbuf4));
			fifo_init(m_dsc[i].tx_fifo, tbuf4, sizeof(uint8_t), sizeof(tbuf4));
#endif
		}

		psp_serial_rx_int_en(i, true);
		psp_serial_tx_int_en(i, true);
	}


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
    if(m_dsc[id].dsc != NULL) {
        /*The fifo is used in the interrupt so disable interrupts*/
        psp_serial_tx_int_en(id, 0);

        if((m_dsc[id].dsc->S1 & UART_S1_TC_MASK) != 0) {
        	m_dsc[id].dsc->D = tx;
        } else {
            bool fifo_ret;
            fifo_ret = fifo_push(&m_dsc[id].tx_fifo, &tx);
            /*Show the fifo become full so not all bytes are buffered*/
            if(fifo_ret == false) {
                res = HW_RES_FULL;
            }
        }

        psp_serial_tx_int_en(id, 1);

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
    if(m_dsc[id].dsc == NULL) return HW_RES_DIS;

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
	if(m_dsc[id].dsc == NULL) return HW_RES_NOT_EX;

    hw_res_t res = HW_RES_OK;

    uint32_t div = CLOCK_PERIPH / baud / 16U;         /* baud divisor=clk/baud/16*/
    if(div == 0) div = 1;
	m_dsc[id].dsc->BDL = 11;//div;

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
//    if(m_dsc[id].UxMODE != NULL) {
//        fifo_clear(&m_dsc[id].rx_fifo);
//    } else {
//        res = HW_RES_DIS;
//    }
    
    return res;
}

#if SERIAL_MODULE_EN(1)
void UART1_IRQHandler (void) {
	irq_generic(HW_SERIAL1);
}
#endif

#if SERIAL_MODULE_EN(2)
void UART2_IRQHandler (void) {
	irq_generic(HW_SERIAL2);
}
#endif

/***********************
 *   STATIC FUNCTIONS
 ***********************/

static void irq_generic(serial_t id)
{
	if(m_dsc[id].dsc == NULL) return;

	if(m_dsc[id].dsc->S1 & UART_S1_RDRF_MASK) {  /* Wait for received buffer to be full */
		uint8_t recieve;
		(void) m_dsc[id].dsc->S1;          /* Read UART_S1 register to clear RDRF (after reading data) */
		recieve = m_dsc[id].dsc->D;         /* Read received data*/
		//There is space in the buffer (not full)
		if(fifo_get_free(&m_dsc[id].rx_fifo) != 0) {
			fifo_push(&m_dsc[id].rx_fifo, &recieve);
		}
	}

	else if(m_dsc[id].dsc->S1 & UART_S1_TDRE_MASK) {
		uint8_t tx_char;
		(void)m_dsc[id].dsc->S1;                    /* Read UART_S1 register to clear TDRE */
		if(fifo_pop(&m_dsc[id].tx_fifo, &tx_char) != false) {
			m_dsc[id].dsc->D = tx_char;                      /* Send data */
		} else {
			psp_serial_tx_int_en(id, false);
		}
	}
}

static void psp_serial_rx_int_en(serial_t id, bool en)
{
	if(m_dsc[id].dsc == NULL) return;

	if(en == false) m_dsc[id].dsc->C2 &= ~(UART_C2_RIE_MASK);
	else m_dsc[id].dsc->C2 |= UART_C2_RIE_MASK;
}

static void psp_serial_tx_int_en(serial_t id, bool en)
{
	if(m_dsc[id].dsc == NULL) return;

	if(en == false) m_dsc[id].dsc->C2 &= ~(UART_C2_TIE_MASK);
	else m_dsc[id].dsc->C2 |= UART_C2_TIE_MASK;
}

#endif
