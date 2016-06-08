/**
 * @file ads1256.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_ADS1256_M != 0

#include <string.h>
#include "misc/os/tick.h"
#include "hw/per/spim.h"
#include "hw/per/io.h"
#include "ads1256_m.h"

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
typedef struct
{
    union
    {
        uint8_t bytes[sizeof(int32_t)];
        int32_t result;
    };
}ads1256_res_t;

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
 * Initialize all ADS1256 parallelly
 */
void ads1256_m_init(void)
{
    
    io_set_pin_dir(ADS1256_M_DRDY_PORT, ADS1256_M_DRDY_PIN, IO_DIR_IN);
    
    uint8_t tx_buf[16];
    uint8_t rx_buf[64];
    memset(tx_buf,0,16);
    
    spim_cs_dis();
    tick_wait_ms(1);
    
    spim_cs_en();
    tick_wait_ms(1);
    
    /*Reset*/
    tx_buf[0] = CMD_RESET;
    spim_xchg(tx_buf, rx_buf, 1);  /*Send the command*/
    
    tick_wait_ms(100);
    
    
    /*Set buffer enable*/
    tx_buf[0] = CMD_WREG | REG_STATUS;  /*Write STATUS reg.*/
    tx_buf[1] = 0;                      /*Write 1 byte*/
    tx_buf[2] = ADS1256_M_BUFEN << BIT_POS_BUFEN;    /*STATUS: Enable the buffer*/    
    spim_xchg(tx_buf, rx_buf, 3);  /*Send the command*/
 
    /*Set the PGA and sample rate*/
    tx_buf[0] = CMD_WREG | REG_ADCON;  /*Write ADCON reg.*/
    tx_buf[1] = 1;                     /*Write 2 byte*/
    tx_buf[2] = ADS1256_M_PGA << BIT_POS_PGA;    /*ADCON: Set the PGA and CLK*/
    tx_buf[2] |= ADS1256_M_CLK_OUT << BIT_POS_CLK;    
    tx_buf[3] = ADS1256_M_SAMPLE_RATE;           /*DRATE: Set sample rate*/    
    spim_xchg(tx_buf, rx_buf, 4);  /*Send the command*/
    tick_wait_us(5);
  
    /*Start self calibation*/
    tx_buf[0] = CMD_SELFCAL;  /*offset and gain*/    
    spim_xchg(tx_buf, rx_buf, 1);  /*Send the command*/
    tick_wait_ms(1000);  /*Wait a lot for salf cal.*/
    
    
    /*Set the MUX to 0 and 1 ch*/
    tx_buf[0] = CMD_WREG | REG_MUX;    /*Write MUX reg.*/
    tx_buf[1] = 0;                     /*Write 1 byte*/
    tx_buf[2] = 0x01;                  /*Select ch 0 and 1*/
    spim_xchg(tx_buf, rx_buf, 3);  /*Send the command*/
    tick_wait_us(5);
    
    /*Send a SYNC command*/
    tx_buf[0] = CMD_SYNC;
    spim_xchg(tx_buf, rx_buf, 1);  /*Send the command*/
    tick_wait_us(5);
    
    /*Send a WAKEUP command*/
    tx_buf[0] = CMD_WAKEUP;
    spim_xchg(tx_buf, rx_buf, 1);  /*Send the command*/
    tick_wait_us(5);
    
    /*Send Read data continously Command*/
    tx_buf[0] = CMD_RDATAC;
    spim_xchg(tx_buf, rx_buf, 1);  /*Send the command*/
    tick_wait_us(10);
}

/**
 * Read all ADS1256 parallelly
 * @return the array of the read 24 bit data(as int32_t numbers)
 */
void ads1256_m_read(int32_t * meas_a)
{
    uint8_t tx_buf[8] = {0};
    uint8_t rx_buf[32] = {0};
    ads1256_res_t ads_res[SPIM_IN_NUM];

#if ADS1256_M_SW_TRIG != 0
    d_rdy = io_get_pin(ADS1256_M_DRDY_PORT, ADS1256_M_DRDY_PIN);
    
    /*Wait for a falling edge On DRDY*/
    while(d_rdy == 0) {
        d_rdy = io_get_pin(ADS1256_M_DRDY_PORT, ADS1256_M_DRDY_PIN);
    }    
    
    while(d_rdy != 0) {
        d_rdy = io_get_pin(ADS1256_M_DRDY_PORT, ADS1256_M_DRDY_PIN);
    }    
#endif
    spim_cs_en();
    
    tx_buf[0] = CMD_WAKEUP;    /*Dummy*/
    tx_buf[1] = CMD_WAKEUP;    /*Dummy*/
    tx_buf[2] = CMD_WAKEUP;    /*Dummy*/
    spim_xchg(tx_buf, rx_buf, 3);  /*Read the data*/
    
    spim_cs_dis();
    
    /*Put the bytes in ads_res struct*/
    uint8_t i;
    for(i = 0; i < SPIM_IN_NUM; i++)
    {
        /*Craete a 32 number from the 24 bit result*/
        /*Correct the byte order too*/
        ads_res[i].bytes[0] = 0;
        ads_res[i].bytes[1] = rx_buf[i + 2 * SPIM_IN_NUM];
        ads_res[i].bytes[2] = rx_buf[i + SPIM_IN_NUM];
        ads_res[i].bytes[3] = rx_buf[i];
        
        /*Normalize the result (32 bit -> 24 bit)*/
        ads_res[i].result = (int32_t)ads_res[i].result >> 8;
    }
    
    /*Copy the result to meas_a*/
    memcpy(meas_a, ads_res, sizeof(ads_res));
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
