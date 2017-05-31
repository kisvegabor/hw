/**
 * @file psp_io.h
 * 
 */

#ifndef PSP_IO_H
#define PSP_IO_H

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_IO != 0

#include "../io.h"

/*Add MCU specific header files to support low level IO operations*/
#if PSP_PIC32MX != 0
#include <xc.h>
#elif PSP_PIC32MZ != 0
#include <xc.h>
#elif PSP_PIC24F_33F != 0
#include <xc.h>
#elif PSP_KEA != 0
#include "derivative.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void psp_io_init(void);
volatile unsigned int psp_io_rd_port(io_port_t port);
void psp_io_wr_port(io_port_t port, volatile unsigned int value);
volatile unsigned int psp_io_rd_dir(io_port_t port);
void psp_io_wr_dir(io_port_t port, volatile unsigned int value);

/**********************
 *      MACROS
 **********************/

#endif

#endif
