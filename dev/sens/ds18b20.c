/**
 * @file ds18b20.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "ds18b20.h"
#if USE_DS18B20 != 0

#include "../../per/tick.h"
#include "../../per/io.h"
#include "../../per/psp/psp_io.h"

/*********************
 *      DEFINES
 *********************/
//commands
#define DS18B20_CMD_CONVERTTEMP 0x44
#define DS18B20_CMD_RSCRATCHPAD 0xbe
#define DS18B20_CMD_WSCRATCHPAD 0x4e
#define DS18B20_CMD_CPYSCRATCHPAD 0x48
#define DS18B20_CMD_RECEEPROM 0xb8
#define DS18B20_CMD_RPWRSUPPLY 0xb4
#define DS18B20_CMD_SEARCHROM 0xf0
#define DS18B20_CMD_READROM 0x33
#define DS18B20_CMD_MATCHROM 0x55
#define DS18B20_CMD_SKIPROM 0xcc
#define DS18B20_CMD_ALARMSEARCH 0xec

//decimal conversion table
#define DS18B20_DECIMALSTEPS_9BIT  5000 //0.5
#define DS18B20_DECIMALSTEPS_10BIT 2500 //0.25
#define DS18B20_DECIMALSTEPS_11BIT 1250 //0.125
#define DS18B20_DECIMALSTEPS_12BIT 625  //0.0625
#define DS18B20_DECIMALSTEPS DS18B20_DECIMALSTEPS_12BIT

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static inline uint8_t pin_rd(ds18b20_bus_t bus);
static inline void pin_pull(ds18b20_bus_t bus);
static inline void pin_rel(ds18b20_bus_t bus);
static inline void pin_in(ds18b20_bus_t bus);
static inline void pin_out(ds18b20_bus_t bus);
static uint8_t ds18b20_reset(ds18b20_bus_t bus);
static void ds18b20_writebit(ds18b20_bus_t bus, uint8_t data);
static uint8_t ds18b20_readbit(ds18b20_bus_t bus);
static void ds18b20_writebyte(ds18b20_bus_t bus, uint8_t byte);
static uint8_t ds18b20_readbyte(ds18b20_bus_t bus);
static void delay_us(uint16_t x);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void ds18b20_init(void)
{
    
}

void ds18b20_start_conv(ds18b20_bus_t bus)
{
	ds18b20_reset(bus); 
	ds18b20_writebyte(bus, DS18B20_CMD_SKIPROM); //skip ROM
	ds18b20_writebyte(bus, DS18B20_CMD_CONVERTTEMP); //start temperature conversion
}

int16_t ds18b20_read_result(ds18b20_bus_t bus)
{
    if(ds18b20_readbit(bus) == 0) return INT16_MIN; //wait until conversion is complete
	
    uint8_t temperature[2];
	int8_t digit;
	uint16_t decimal;
	uint16_t temp_meas = 0;
    
	ds18b20_reset(bus); 
	ds18b20_writebyte(bus, DS18B20_CMD_SKIPROM);     
	ds18b20_writebyte(bus, DS18B20_CMD_RSCRATCHPAD); 
    
	/*read 2 byte from scratchpad*/
	temperature[0] = ds18b20_readbyte(bus);
	temperature[1] = ds18b20_readbyte(bus);

	ds18b20_reset(bus); 
    
	//store temperature integer digits
	digit = temperature[0]>>4;
	digit |= (temperature[1]&0x7)<<4;

	//store temperature decimal digits
	decimal = temperature[0]&0xf;
	decimal *= DS18B20_DECIMALSTEPS;

	temp_meas = digit * 1000 + decimal / 10;

	return temp_meas;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static uint8_t ds18b20_reset(ds18b20_bus_t bus)
{
	uint8_t i;

    pin_pull(bus);
	tick_wait_us(480);

	pin_rel(bus);
	tick_wait_us(60);

	i =  pin_rd(bus);
	tick_wait_us(420);

	return i; /*0=ok, 1=error*/
}

/*
 * write one bit
 */
static void ds18b20_writebit(ds18b20_bus_t bus, uint8_t data)
{
    pin_pull(bus);
 	tick_wait_us(1);

	/*if we want to write 1, release the line (if not will keep low)*/
	if(data)  pin_rel(bus);
    
	tick_wait_us(60);
    pin_rel(bus);
}

/*
 * read one bit
 */
static uint8_t ds18b20_readbit(ds18b20_bus_t bus)
{
	volatile uint8_t data=0;

    pin_pull(bus);
	tick_wait_us(1);

    pin_rel(bus);
	tick_wait_us(14);

	/*read the value*/
	if(pin_rd(bus)) data=1;

	tick_wait_us(45);
	return data;
}

/*
 * write one byte
 */
static void ds18b20_writebyte(ds18b20_bus_t bus, uint8_t byte)
{
	uint8_t i = 8;
	while(i--){
		ds18b20_writebit(bus, byte & 0x01);
		byte >>= 1;
	}
}

/*
 * read one byte
 */
static uint8_t ds18b20_readbyte(ds18b20_bus_t bus)
{
	uint8_t i=8, n=0;
	while(i--){
		n >>= 1;
		n |= (ds18b20_readbit(bus) << 7);
	}
	return n;
}


static inline uint8_t pin_rd(ds18b20_bus_t bus)
{
    uint8_t pin_state;
    switch(bus) {
        case DS18B20_BUS_0: DS18B20_BUS0_RD(&pin_state); break;
        case DS18B20_BUS_1: DS18B20_BUS1_RD(&pin_state); break;
        case DS18B20_BUS_2: DS18B20_BUS2_RD(&pin_state); break;
        case DS18B20_BUS_3: DS18B20_BUS3_RD(&pin_state); break;
    }
    
    return pin_state;
}

static inline void pin_pull(ds18b20_bus_t bus)
{
    switch(bus) {
        case DS18B20_BUS_0: DS18B20_BUS0_PULL; break;
        case DS18B20_BUS_1: DS18B20_BUS1_PULL; break;
        case DS18B20_BUS_2: DS18B20_BUS2_PULL; break;
        case DS18B20_BUS_3: DS18B20_BUS3_PULL; break;
    }    
}

static inline void pin_rel(ds18b20_bus_t bus)
{
    
    switch(bus) {
        case DS18B20_BUS_0: DS18B20_BUS0_REL; break;
        case DS18B20_BUS_1: DS18B20_BUS1_REL; break;
        case DS18B20_BUS_2: DS18B20_BUS2_REL; break;
        case DS18B20_BUS_3: DS18B20_BUS3_REL; break;
    }
}
#endif

