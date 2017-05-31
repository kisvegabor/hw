/**
 * @file gpio.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"

#if USE_IO != 0 && PSP_KEA != 0

#include "derivative.h" /* include peripheral declarations SSKEAZ128M4 */

#include "hw/hw.h"
#include "hw/per/io.h"
#include "hw/per/psp/psp_io.h"
#include <stddef.h>
#include <stdint.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
volatile uint8_t * regmap[IO_PORT_NUM];

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the IO ports
 */
void psp_io_init(void)
{ 

	/*Enable input buffers*/
	GPIOA_PIDR = 0;
	GPIOB_PIDR = 0;
	GPIOC_PIDR = 0;
}

/**
 * Read a port
 * @param port id of a port from io_port_t enum
 * @return the value of the port
 */
volatile unsigned int psp_io_rd_port(io_port_t port)
{
	uint8_t reg_port;
	reg_port = port >> 2;

	uint8_t pos_port;
	pos_port = (port & 0x03) * 8;

	uint8_t result = 0;

	switch(reg_port) {
		case 0:
			result = (GPIOA_PDIR >> pos_port) & 0xFF;
			break;
		case 1:
			result = (GPIOB_PDIR >> pos_port) & 0xFF;
			break;
		case 2:
			result = (GPIOC_PDIR >> pos_port) & 0xFF;
			break;
		default:
			result = 0;
	}

	return result;
}

/**
 * Write a port
 * @param port id of port from io_port_t
 * @param value value to write
 */
void psp_io_wr_port(io_port_t port, volatile unsigned int value)
{
	uint8_t reg_port;
	reg_port = port >> 2;

	uint8_t pos_port;
	pos_port = (port & 0x03) * 8;

	value = value & 0xFF;

	switch(reg_port) {
		case 0:
			GPIOA_PDOR &=  (uint32_t)~((uint32_t)0xFF << pos_port);
			GPIOA_PDOR |=(uint32_t) value << pos_port;
			break;

		case 1:
			GPIOB_PDOR &=  (uint32_t)~((uint32_t)0xFF << pos_port);
			GPIOB_PDOR |= (uint32_t)value << pos_port;
			break;

		case 2:
			GPIOC_PDOR &=  (uint32_t)~((uint32_t)0xFF << pos_port);
			GPIOC_PDOR |= (uint32_t)value << pos_port;
			break;

		default:
			/*Do nothing*/
			break;
	}
}

/**
 * Read the direction register of a port
 * @param port d of port from io_port_t
 * @return the value of the direction register of a port
 */
volatile unsigned int psp_io_rd_dir(io_port_t port)
{
	uint8_t reg_port;
	reg_port = port >> 2;

	uint8_t pos_port;
	pos_port = (port & 0x03) * 8;

	uint8_t result = 0;

	switch(reg_port) {
		case 0:
			result = (GPIOA_PDDR >> pos_port) & 0xFF;
			break;
		case 1:
			result = (GPIOB_PDDR >> pos_port) & 0xFF;
			break;
		case 2:
			result = (GPIOC_PDDR >> pos_port) & 0xFF;
			break;
		default:
			result = 0;
	}

	if(IO_DIR_IN != 0) result = ~result;

	return result;
}

/**
 * Write the direction register of a port
 * @param port id of port from io_port_t
 * @param value value to write
 */
void psp_io_wr_dir(io_port_t port, volatile unsigned int value)
{
	uint8_t reg_port;
	reg_port = port >> 2;

	uint8_t pos_port;
	pos_port = (port & 0x03) * 8;

	value = value & 0xFF;

	if(IO_DIR_IN != 0) value = ~value;

	switch(reg_port) {
		case 0:
			GPIOA_PDDR &= (uint32_t)~((uint32_t)0xFF << pos_port);
			GPIOA_PDDR |= (uint32_t)value << pos_port;
			break;

		case 1:
			GPIOB_PDDR &=  (uint32_t)~((uint32_t)0xFF << pos_port);
			GPIOB_PDDR |= (uint32_t)value << pos_port;
			break;

		case 2:
			GPIOC_PDDR &=  (uint32_t)~((uint32_t)0xFF << pos_port);
			GPIOC_PDDR |= (uint32_t)value << pos_port;
			break;

		default:
			/*Do nothing*/
			break;
	}

}


/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
