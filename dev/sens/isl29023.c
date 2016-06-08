/**
 * @file isl29023.c
 * Infrared and ambient light sensor (I2C)
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_ISL29023 != 0

#include "isl29023.h"
#include "hw/per/i2c.h"
#include "misc/os/tick.h"

/*********************
 *      DEFINES
 *********************/
#define ISL29023_I2C_ADDRESS   0b1000100
#define REG_COMM1	     0x00
#define REG_COMM2	     0x01
#define REG_DATA_LOW	 0x02
#define REG_DATA_HIGH    0x03
#define REG_TEST_ADDR    0x08
#define CMD_POWER_DOWN   0x00
#define CMD_ONCE_AL	     0x20   // once measure than sleep
#define CMD_ONCE_IR	     0x40   // once measure than sleep
#define CMD_CONT_AL	     0xA0   //continuos measure
#define CMD_CONT_IR	     0xC0   //continuos measure
#define CMD_RANGE_1000   0x00   // VAL max =1000 lux
#define CMD_RANGE_4000   0x01   // VAL max =4000 lux
#define CMD_RANGE_16000  0x02   // VAL max =16000 lux
#define CMD_RANGE_64000  0x03   // VAL max =64000 lux
#define CMD_RES16	    (0x00 << 2) //16 bit ADC
#define CMD_RES12	    (0x01 << 2) //12 bit ADC
#define CMD_RES8	    (0x02 << 2) //8 bit ADC
#define CMD_RES4	    (0x03 << 2) //4 bit ADC
#define CMD_TEST_VALUE   0x00

#if ISL29023_RES == 16
#define ISL29023_RES_CMD CMD_RES16
#define ISL29023_BUSY_MS 100
#elif ISL29023_RES == 12
#define ISL29023_RES_CMD CMD_RES12
#define ISL29023_BUSY_MS 50
#elif ISL29023_RES == 8
#define ISL29023_RES_CMD CMD_RES8
#define ISL29023_BUSY_MS 2
#elif ISL29023_RES == 4
#define ISL29023_RES_CMD CMD_RES4
#define ISL29023_BUSY_MS 2
#else
#error "ISL29023: Invalid reolution (ISL29023_RES can be 16, 12, 8 or 4 )"
#endif

#if ISL29023_RANGE == 1000
#define ISL29023_RANGE_CMD CMD_RANGE_1000
#elif ISL29023_RANGE == 4000
#define ISL29023_RANGE_CMD CMD_RANGE_4000
#elif ISL29023_RANGE == 16000
#define ISL29023_RANGE_CMD CMD_RANGE_16000
#elif ISL29023_RANGE == 64000
#define ISL29023_RANGE_CMD CMD_RANGE_64000
#else
#error "ISL29023: Invalid range (ISL29023_RANGE can be 1000, 4000, 16000 or 64000 )"
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static uint32_t al_start_time = 0;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
 * Initialize the ISL29023 sensor
 */
void isl29023_init(void)
{
    uint8_t buf[3];
    uint8_t cmd;
    cmd = REG_TEST_ADDR;
    i2c_read(ISL29023_I2C_DRV, ISL29023_I2C_ADDRESS, cmd, buf, 1);

    if (buf[0] != CMD_TEST_VALUE) {
        return; 
    }
    
	/*set mode*/
	buf[0]= REG_COMM1;
	buf[1]= CMD_POWER_DOWN;
	i2c_send(ISL29023_I2C_DRV, ISL29023_I2C_ADDRESS,buf,2);
	
    /*set measure range and resolution*/
	buf[0]= REG_COMM2;
	buf[1]= ISL29023_RANGE_CMD | ISL29023_RES_CMD;
	i2c_send(ISL29023_I2C_DRV, ISL29023_I2C_ADDRESS, buf, 2);
}

/**
 * Start an ambient light measurement
 * @return I2C error: HW_RES_OK or any error from hw_res_t enum
 */
hw_res_t isl29023_start_al(void)
{
    uint8_t  buff[3];
    
    //set mode
    buff[0]= REG_COMM1;   //select command register
    buff[1]= CMD_ONCE_AL; //select once light conversation
    
    hw_res_t res;
    res = i2c_send(ISL29023_I2C_DRV, ISL29023_I2C_ADDRESS, buff,2);
    if(res == HW_RES_OK) {
        al_start_time = tick_get();
    }
    
    return res;
}

/**
 * Check the started measure is still in progress or ready
 * @return true: device busy, measure in progress; false: measure is ready
 */
bool isl29023_busy(void)
{
    if(tick_elaps(al_start_time) >= ISL29023_BUSY_MS) return false;
    else return true;
}

/**
 * Read the measured ambient light value
 * @param meas pointer to a variable to store the result
 * @return I2C error: HW_RES_OK or any error from hw_res_t enum
 */
hw_res_t isl29023_read_al (uint32_t * meas)
{
    *meas = 0;
    if(isl29023_busy() != false) return HW_RES_NOT_RDY;
    
    /*Read the data*/
    hw_res_t res;
    res = i2c_read(ISL29023_I2C_DRV, ISL29023_I2C_ADDRESS, REG_DATA_LOW, meas, 2);
    if(res != HW_RES_OK) {
        *meas = 0;
        return res;
    }
    
    *meas = (uint32_t)((uint32_t) ISL29023_REAL_RANGE * (*meas)) >> ISL29023_RES;
    
    return HW_RES_OK;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /* USE_ISL29023 != 0*/