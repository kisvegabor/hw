/**
 * @file ads1256.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_ADS1256 != 0

#include <string.h>
#include "hw/per/sys.h"
#include "hw/per/spi.h"
#include "ads1256.h"

/*********************
 *      DEFINES
 *********************/

/*ADS1256 registers*/
#define REG_STATUS      0x00
#define REG_MUX         0x01
#define REG_ADCON       0x02
#define REG_DRATE       0x03
#define REG_IO          0x04
#define REG_OFC0        0x05
#define REG_OFC1        0x06
#define REG_OFC2        0x07
#define REG_FSC0        0x08
#define REG_FSC1        0x09
#define REG_FSC2        0x0A

/*ADS1256 Commands*/
#define CMD_WAKEUP      0x00    /*Completes SYNC and Exits Standby Mode*/
#define CMD_RDATA       0x01    /*Read Data*/
#define CMD_RDATAC      0x03    /*Read Data Continuously*/
#define CMD_SDATAC      0x0F    /*Stop Read Data Continuously*/
#define CMD_RREG        0x10    /*Read from REG rrr, (2nd cmd 0000 nnnn)*/
#define CMD_WREG        0x50    /*Write to REG rrr, (2nd cmd 0000 nnnn)*/
#define CMD_SELFCAL     0xF0    /*Offset and Gain Self-Calibration*/
#define CMD_SELFOCAL    0xF1    /*Offset Self-Calibration*/
#define CMD_SELFGCAL    0xF2    /*Gain Self-Calibration*/
#define CMD_SYSOCAL     0xF3    /*System Offset Calibration*/
#define CMD_SYSGCAL     0xF4    /*System Gain Calibration*/
#define CMD_SYNC        0xFC    /*Synchronize the A/D Conversion*/
#define CMD_STANDBY     0xFD    /*Begin Standby Mode*/
#define CMD_RESET       0xFE    /*Reset to Power-Up Values*/
#define CMD_WAKEUP2     0xFF    /*Completes SYNC and Exits Standby Mode*/

/*Bit position*/
#define BIT_POS_PGA     0
#define BIT_POS_BUFEN   1
#define BIT_POS_CLK     5

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
 * Initialize the ADS1256
 */
void ads1256_init(void)
{
    io_set_pin_dir(ADS1256_DRDY_PORT, ADS1256_DRDY_PIN, IO_DIR_IN);
    
    uint8_t buf[16];
    memset(buf,0,16);
    
    spi_cs_dis(ADS1256_DRV);
    tick_wait_ms(1);
    
    spi_cs_en(ADS1256_DRV);
    tick_wait_ms(1);
    
    /*Reset*/
    buf[0] = CMD_RESET;
    spi_xchg(ADS1256_DRV, buf, 1);  /*Send the command*/
    tick_wait_ms(100);
    
    
    /*Set buffer enable*/
    buf[0] = CMD_WREG | REG_STATUS;  /*Write STATUS reg.*/
    buf[1] = 0;                      /*Write 1 byte*/
    buf[2] = ADS1256_BUFEN << BIT_POS_BUFEN;    /*STATUS: Enable the buffer*/    
    spi_xchg(ADS1256_DRV, buf, 3);  /*Send the command*/
 
    /*Set the PGA and sample rate*/
    buf[0] = CMD_WREG | REG_ADCON;  /*Write ADCON reg.*/
    buf[1] = 1;                     /*Write 2 byte*/
    buf[2] = ADS1256_PGA << BIT_POS_PGA;    /*ADCON: Set the PGA and CLK*/
    buf[2] |= ADS1256_CLK_OUT << BIT_POS_CLK;    
    buf[3] = ADS1256_SAMPLE_RATE;           /*DRATE: Set sample rate*/    
    spi_xchg(ADS1256_DRV, buf, 4);  /*Send the command*/
    tick_wait_us(5);
  
    /*Start self calibation*/
    buf[0] = CMD_SELFCAL;  /*offset and gain*/    
    spi_xchg(ADS1256_DRV, buf, 1);  /*Send the command*/
    tick_wait_ms(1000);  /*Wait a lot for salf cal.*/
    
    
    /*Set the MUX to 0 and 1 ch*/
    buf[0] = CMD_WREG | REG_MUX;    /*Write MUX reg.*/
    buf[1] = 0;                     /*Write 1 byte*/
    buf[2] = 0x01;                  /*Select ch 0 and 1*/
    spi_xchg(ADS1256_DRV, buf, 3);  /*Send the command*/
    tick_wait_us(5);
    
    /*Send a SYNC command*/
    buf[0] = CMD_SYNC;
    spi_xchg(ADS1256_DRV, buf, 1);  /*Send the command*/
    tick_wait_us(5);
    
    
    /*Send a WAKEUP command*/
    buf[0] = CMD_WAKEUP;
    spi_xchg(ADS1256_DRV, buf, 1);  /*Send the command*/
    tick_wait_us(5);
    
    /*Send Read data continously Command*/
    buf[0] = CMD_RDATAC;
    spi_xchg(ADS1256_DRV, buf, 1);  /*Send the command*/
    tick_wait_us(10);
    
}

/**
 * Read the ADS1256
 * @return the read 24 bit number
 */
int32_t ads1256_read(void)
{
   
    uint8_t buf[16] = {0};
 
#if ADS1256_SW_TRIG != 0
  d_rdy = io_get_pin(ADS1256_DRDY_PORT, ADS1256_DRDY_PIN);
    
    /*Wait for a falling edge On DRDY*/
    while(d_rdy == 0) {
        d_rdy = io_get_pin(ADS1256_DRDY_PORT, ADS1256_DRDY_PIN);
    }    
    
    while(d_rdy != 0) {
        d_rdy = io_get_pin(ADS1256_DRDY_PORT, ADS1256_DRDY_PIN);
    }      
#endif
    
    spi_cs_en(ADS1256_DRV);
    
    
    /*Start from buf[1] hence it will look lika a 32 bit number with empty LSB*/
    buf[1] = CMD_WAKEUP;    /*Dummy*/
    buf[2] = CMD_WAKEUP;    /*Dummy*/
    buf[3] = CMD_WAKEUP;    /*Dummy*/
    spi_xchg(ADS1256_DRV, &buf[1], 3);  /*Read the data*/
    
    spi_cs_dis(ADS1256_DRV);
    
    /*Swap the bytes to get Little Endian byte order*/
    /*Swap buf[3] and buf[1], use buf[5] as temp. reg.*/
    buf[5] = buf[1];
    buf[1] = buf[3];
    buf[3] = buf[5];
    
    /*Convert the read data to int32_t*/
    int32_t meas = 0;
    memcpy(&meas, buf, 4);

    meas = meas >> 8;
    
    return meas;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
