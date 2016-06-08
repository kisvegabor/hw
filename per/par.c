/**
 * @file par.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_PARALLEL != 0

#include "psp/psp_par.h"
#include "par.h"
#include "io.h"
#include "psp/psp_io.h"
#include "hw/per/tick.h"

/*********************
 *      DEFINES
 *********************/
#if PAR_SW != 0
#define REPEATE8(cmd) {cmd; cmd; cmd; cmd; cmd; cmd; cmd; cmd;}
#define BATCH_COM      64

#ifndef PARSW_WR_STROBE
#define PARSW_WR_STROBE() par_sw_wr_strobe();
#endif


#ifndef PARSW_WR_DATA
#define PARSW_WR_DATA(data_p) {io_set_port(PARSW_DATA_PORT, *data_p); \
                               data_p++; \
                               par_sw_wr_strobe();}
#endif

#define PARSW_SLOW_WR_DATA(data_p) {io_set_port(PARSW_DATA_PORT, *data_p); \
                                    data_p++; \
                                    par_sw_slow_wr_strobe();}

#endif
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
#if PAR_SW != 0
static void par_sw_wr_array(uint32_t adr, const uint16_t * data_p, uint32_t length);
static void par_sw_fill(uint32_t adr, uint16_t data, uint32_t length);
static void par_sw_slow_wr_strobe(void);
#ifndef PARSW_WR_STROBE  
static void par_sw_wr_strobe(void);
#endif
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
static uint8_t slow_mode = 0;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the parallel port
 */
void par_init(void)
{
    
    /*Init the parallel hardware*/
    psp_par_init();
    
    io_set_pin_dir(PAR_CS1_PORT, PAR_CS1_PIN, IO_DIR_OUT);
    io_set_pin(PAR_CS1_PORT, PAR_CS1_PIN, 1);
    io_set_pin_dir(PAR_CS2_PORT, PAR_CS2_PIN, IO_DIR_OUT);
    io_set_pin(PAR_CS2_PORT, PAR_CS2_PIN, 1);
    
#if PAR_SW != 0
    io_set_pin_dir(PARSW_RD_PORT, PARSW_RD_PIN, IO_DIR_OUT);
    io_set_pin(PARSW_RD_PORT, PARSW_RD_PIN, 1);
    io_set_pin_dir(PARSW_WR_PORT, PARSW_WR_PIN, IO_DIR_OUT);
    io_set_pin(PARSW_WR_PORT, PARSW_WR_PIN, 1);
    
    io_set_port_dir(PARSW_ADR_PORT, IO_DIR_OUT);
    io_set_port(PARSW_ADR_PORT, 0);
    io_set_port_dir(PARSW_DATA_PORT, IO_DIR_OUT);
    io_set_port(PARSW_DATA_PORT, 0);
#endif
}

/**
 * Set the wait cycles. 
 * @param wait length of a wr/rd strobe in clock cycles 
 */
void par_set_wait_time(uint8_t wait)
{
    if(wait == PAR_SLOW) slow_mode = 1;
    else slow_mode = 0;
    
    psp_par_set_wait_time(wait);
}

/**
 * Pull down a Chip Select
 * @param cs a Chip Select from par_cs_t
 */
void par_cs_en(par_cs_t cs)
{
    switch(cs)
    {
        case PAR_CS1:
            io_set_pin(PAR_CS1_PORT, PAR_CS1_PIN, 0);
            break;
            
        case PAR_CS2:
            io_set_pin(PAR_CS2_PORT, PAR_CS2_PIN, 0);
            break;
        default:
            break;
    }
}


/**
 * Release a Chip Select
 * @param cs a Chip Select from par_cs_t
 */
void par_cs_dis(par_cs_t cs)
{   
    switch(cs)
    {
        case PAR_CS1:
            io_set_pin(PAR_CS1_PORT, PAR_CS1_PIN, 1);
            break;
            
        case PAR_CS2:
            io_set_pin(PAR_CS2_PORT, PAR_CS2_PIN, 1);
            break;
        default:           
            break;
    }
}

/**
 * Write 1 word to the parallel port
 * @param data the word to write 
 */
void par_wr(uint16_t data)
{

#if PAR_SW != 0
    par_sw_wr_array(0, &data, 1);
#else
    psp_par_wr_array(0, &data, 1);
#endif
}

/**
 * Write an array to the parallel port
 * @param data_p pointer to the data to write
 * @param size number of element in the array 
 */
void par_wr_array(uint16_t * data_p, uint32_t size)
{
#if PAR_SW != 0
    par_sw_wr_array(0, data_p, size);
#else
    psp_par_wr_array(0, data_p, size);
#endif
}

/**
 * Write a word to the parallel port multiply times
 * @param data a word to write
 * @param mult the number of repeats 
 */
void par_wr_mult(uint16_t  data, uint32_t mult)
{
#if PAR_SW != 0
    par_sw_fill(0, data, mult);
#else
    uint32_t i;
    for(i = 0; i < mult; i++) {
        psp_par_wr_array(0, &data, 1);
    }
#endif 
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#if PAR_SW != 0
static void par_sw_wr_array(uint32_t adr, const uint16_t * data_p, uint32_t length)
{  
    uint32_t i;    
    uint32_t len_mod = length / BATCH_COM;
    
    /*In slow mode write data slowly*/
    if(slow_mode != 0) {
        for(i = 0; i < len_mod; i++) {
            REPEATE8(REPEATE8(PARSW_SLOW_WR_DATA(data_p)));
        }    

        len_mod = length % BATCH_COM;
        for(i = 0; i < len_mod; i++) {
            PARSW_SLOW_WR_DATA(data_p)
        }
    } else { 
        /*In NOT slow mode write with max speed (in sw mode the max is slow too)*/
        
        for(i = 0; i < len_mod; i++) {
            REPEATE8(REPEATE8(PARSW_WR_DATA(*data_p); data_p ++));
        }    

        len_mod = length % BATCH_COM;
        for(i = 0; i < len_mod; i++) {
            PARSW_WR_DATA(*data_p);
            data_p++;
        }
    }
}

static void par_sw_fill(uint32_t adr, uint16_t data, uint32_t length)
{
    uint16_t * data_p = &data; 
    /* Write the first data, it will set the data port */
    PARSW_WR_DATA(*data_p);
    data_p++;
    length --;
    
    uint32_t i;
    uint32_t len_mod = length / BATCH_COM;
    
    if(slow_mode != 0) {
        for(i = 0; i < len_mod; i++) {
            REPEATE8(REPEATE8(par_sw_slow_wr_strobe()));
        }
  
        len_mod = length % BATCH_COM;
   
        for(i = 0; i < len_mod; i++) {
          par_sw_slow_wr_strobe();
        }
    } else {
        for(i = 0; i < len_mod; i++) {
            REPEATE8(REPEATE8(PARSW_WR_STROBE));
        }
  
        len_mod = length % BATCH_COM;
   
        for(i = 0; i < len_mod; i++) {
          PARSW_WR_STROBE;
        }
    }
}
    
    
#ifndef PARSW_WR_STROBE  
/**
 * 
 */
static void par_sw_wr_strobe(void)
{
    io_set_pin(PARSW_WR_PORT, PARSW_WR_PIN, 0);
    io_set_pin(PARSW_WR_PORT, PARSW_WR_PIN, 1);
}
#endif

/**
 * 
 */
static void par_sw_slow_wr_strobe(void)
{
    tick_wait_us(1);
    io_set_pin(PARSW_WR_PORT, PARSW_WR_PIN, 0);
    tick_wait_us(1);
    io_set_pin(PARSW_WR_PORT, PARSW_WR_PIN, 1);
}

#endif
#endif