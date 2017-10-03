/**
 * @file hw_conf.h
 * 
 */

#if 0 /*Remove this to enable the content*/


#ifndef HW_CONF_H
#define HW_CONF_H

/*********************
 *   PERIPHERY CONFIG
 *********************/

/*---------------
 *  PSP select 
 *--------------*/
/* Choose a hardware to enable to corresponding Processor Support Package (psp)
 * See hw/per/psp/ directories */
#define PSP_PIC24F_33F     0
#define PSP_PIC32MX        0
#define PSP_PIC32MZ        0
#define PSP_KEA		       0
#define PSP_PC	       	   0

/*-----------
 *   Clock
 *----------*/
#define CLOCK_CORE       70000000UL /*Hz*/
#define CLOCK_PERIPH     70000000UL /*Hz*/

/*-----------
 *   GPIO 
 *----------*/
#define USE_IO        1
#if USE_IO != 0
/*No settings*/
#endif /*USE_IO*/

/*-----------
 *   Timer 
 *----------*/
#define USE_TMR     1
#if USE_TMR != 0

/*TMR0*/
#define TMR0_EN     1
#if TMR0_EN     !=  0
#define TMR0_PRIO   HW_INT_PRIO_MID
#endif

/*TMR1*/
#define TMR1_EN     0
#if TMR1_EN     !=  0
#define TMR1_PRIO   HW_INT_PRIO_MID
#endif

/*TMR2*/
#define TMR2_EN     0
#if TMR2_EN     !=  0
#define TMR2_PRIO   HW_INT_PRIO_MID
#endif

/*TMR3*/
#define TMR3_EN     0
#if TMR3_EN     !=  0
#define TMR3_PRIO   HW_INT_PRIO_MID
#endif

/*TMR4*/
#define TMR4_EN     0
#if TMR4_EN     !=  0
#define TMR4_PRIO   HW_INT_PRIO_MID
#endif

/*TMR5*/
#define TMR5_EN     0
#if TMR5_EN     !=  0
#define TMR5_PRIO   HW_INT_PRIO_MID
#endif

/*TMR6*/
#define TMR6_EN     0
#if TMR6_EN     !=  0
#define TMR6_PRIO   HW_INT_PRIO_MID
#endif
#endif  /*USE_TMR*/

/*----------------
 *     Tick
 *----------------*/
#define USE_TICK         1
#if USE_TICK != 0
#define TICK_FUNC_NUM 16
#define TICK_US_BASE     5  /*Adjust the 'tick_wait_us' functions */
#define TICK_TIMER		HW_TMR2
#else   /*Without tick a very simple wait functions can be enabled*/
#define TICK_BLOCK_WAIT  1   /*Enable simple blocking wait functions*/
#define TICK_US_BASE     5   /*Adjust the 'tick_wait_us' functions */
#endif /*USE_TICK*/

/*-----------------
 * SERIAL (UART)
 *----------------*/
#define USE_SERIAL        0
#if USE_SERIAL != 0

/*SERIAL0*/
#define SERIAL0_PRIO       HW_INT_PRIO_OFF /*HW_INT_PRIO_OFF to disable module*/
#define SERIAL0_BUF_SIZE   0				 /*0: disable module*/
#define SERIAL0_MODE       (SERIAL_MODE_BASIC)

/*SERIAL1*/
#define SERIAL1_PRIO       HW_INT_PRIO_OFF /*HW_INT_PRIO_OFF to disable module*/
#define SERIAL1_BUF_SIZE   0				 /*0: disable module*/
#define SERIAL1_MODE       (SERIAL_MODE_BASIC)

/*SERAL2*/
#define SERIAL2_PRIO       HW_INT_PRIO_OFF /*HW_INT_PRIO_OFF to disable module*/
#define SERIAL2_BUF_SIZE   0
#define SERIAL2_MODE       (SERIAL_MODE_BASIC)

/*SERIAL3*/
#define SERIAL3_PRIO       HW_INT_PRIO_OFF /*HW_INT_PRIO_OFF to disable module*/
#define SERIAL3_BUF_SIZE   0
#define SERIAL3_MODE       (SERIAL_MODE_BASIC)

/*SERIAL4*/
#define SERIAL4_PRIO       HW_INT_PRIO_OFF /*HW_INT_PRIO_OFF to disable*/
#define SERIAL4_BUF_SIZE   0
#define SERIAL4_MODE       (SERIAL_MODE_BASIC)
#endif /*USE_SERIAL*/

/*-----------
 *   SPI 
 *----------*/
#define USE_SPI         0
#if USE_SPI != 0

/*SPI0*/
#define SPI0_EN        0
#define SPI0_CS1_PORT  IO_PORTX
#define SPI0_CS1_PIN   IO_PINX
#define SPI0_CS2_PORT  IO_PORTX
#define SPI0_CS2_PIN   IO_PINX
#define SPI0_CS3_PORT  IO_PORTX
#define SPI0_CS3_PIN   IO_PINX
#define SPI0_CS4_PORT  IO_PORTX
#define SPI0_CS4_PIN   IO_PINX

/*SPI1*/
#define SPI1_EN        0
#define SPI1_CS1_PORT  IO_PORTX
#define SPI1_CS1_PIN   IO_PINX
#define SPI1_CS2_PORT  IO_PORTX
#define SPI1_CS2_PIN   IO_PINX
#define SPI1_CS3_PORT  IO_PORTX
#define SPI1_CS3_PIN   IO_PINX
#define SPI1_CS4_PORT  IO_PORTX
#define SPI1_CS4_PIN   IO_PINX

/*SPI2*/
#define SPI2_EN        0
#define SPI2_CS1_PORT  IO_PORTX
#define SPI2_CS1_PIN   IO_PINX
#define SPI2_CS2_PORT  IO_PORTX
#define SPI2_CS2_PIN   IO_PINX
#define SPI2_CS3_PORT  IO_PORTX
#define SPI2_CS3_PIN   IO_PINX
#define SPI2_CS4_PORT  IO_PORTX
#define SPI2_CS4_PIN   IO_PINX

/*SPI3*/
#define SPI3_EN        0
#define SPI3_CS1_PORT  IO_PORTX
#define SPI3_CS1_PIN   IO_PINX
#define SPI3_CS2_PORT  IO_PORTX
#define SPI3_CS2_PIN   IO_PINX
#define SPI3_CS3_PORT  IO_PORTX
#define SPI3_CS3_PIN   IO_PINX
#define SPI3_CS4_PORT  IO_PORTX
#define SPI3_CS4_PIN   IO_PINX

/*SPI4*/
#define SPI4_EN        0
#define SPI4_CS1_PORT  IO_PORTX
#define SPI4_CS1_PIN   IO_PINX
#define SPI4_CS2_PORT  IO_PORTX
#define SPI4_CS2_PIN   IO_PINX
#define SPI4_CS3_PORT  IO_PORTX
#define SPI4_CS3_PIN   IO_PINX
#define SPI4_CS4_PORT  IO_PORTX
#define SPI4_CS4_PIN   IO_PINX

/*SPI5*/
#define SPI5_EN        0
#define SPI5_CS1_PORT  IO_PORTX 
#define SPI5_CS1_PIN   IO_PINX
#define SPI5_CS2_PORT  IO_PORTX
#define SPI5_CS2_PIN   IO_PINX
#define SPI5_CS3_PORT  IO_PORTX
#define SPI5_CS3_PIN   IO_PINX
#define SPI5_CS4_PORT  IO_PORTX
#define SPI5_CS4_PIN   IO_PINX

/*SPI_SW*/
#define SPISW_SCK_PORT  IO_PORTX
#define SPISW_SCK_PIN   IO_PINX
#define SPISW_SDO_PORT  IO_PORTX
#define SPISW_SDO_PIN   IO_PINX
#define SPISW_SDI_PORT  IO_PORTX
#define SPISW_SDI_PIN   IO_PINX
#define SPISW_CS1_PORT  IO_PORTX
#define SPISW_CS1_PIN   IO_PINX
#define SPISW_CS2_PORT  IO_PORTX
#define SPISW_CS2_PIN   IO_PINX
#define SPISW_CS3_PORT  IO_PORTX
#define SPISW_CS3_PIN   IO_PINX
#define SPISW_CS4_PORT  IO_PORTX
#define SPISW_CS4_PIN   IO_PINX

#endif  /*USE_SPI*/

/*-----------
 *   I2C 
 *----------*/
#define USE_I2C         0
#if USE_I2C != 0

/*I2C0*/
#define I2C0_BAUD       0 /* 0: disable the module */   

/*I2C1*/
#define I2C1_BAUD       0 /* 0: disable the module */   

/*I2C2*/
#define I2C2_BAUD       0   
#endif /*USE_I2C*/

/*--------------
 *   Parallel 
 *-------------*/
#define USE_PARALLEL     0
#if USE_PARALLEL != 0
#define PAR_CS1_PORT     IO_PORTX
#define PAR_CS1_PIN      IO_PINX
#define PAR_CS2_PORT     IO_PORTX
#define PAR_CS2_PIN      IO_PINX
#define PAR_SW           0
#if PAR_SW == 0     /*Hw par. settings*/
#define PAR_WAITB        1      /*Begin wait cycles (>=1)*/
#define PAR_WAITM        1      /*Middle wait cycles (>=1)*/
#define PAR_WAITE        1      /*End wait cycles (>=1)*/
#else               /*Sw par. settings*/
#define PARSW_DATA_PORT   IO_PORTX
#define PARSW_ADR_PORT    IO_PORTX
#define PARSW_WR_PORT     IO_PORTX
#define PARSW_WR_PIN      IO_PINX
#define PARSW_RD_PORT     IO_PORTX
#define PARSW_RD_PIN      IO_PINX
//#define PARSW_WR_DATA(data) {}  /*Parallel wr data*/
//#define PARSW_RD_DATA(data) {}  /*Parallel rd data*/
//#define PARSW_WR_STROBE {}  /*Parallel wr strobe*/
//#define PARSW_RD_STROBE {}  /*Parallel rd data*/
#endif  /*PAR_SW*/
#endif  /*USE_PARALLEL*/


/*-----------------------------
 *  Internal TFT or PC monitor
 *----------------------------*/
#define USE_TFT     0
#if USE_TFT != 0	
#define TFT_HOR_RES   480
#define TFT_HSL       30     /*HSYNC length*/
#define TFT_HBP       38     /*Hor. Back proach*/
#define TFT_HFP       12     /*Hor. Front proach*/

#define TFT_VER_RES  320            
#define TFT_VSL      3       /*VSYNC length*/
#define TFT_VBP      26      /*Ver. Back proach*/
#define TFT_VFP      3       /*Ver. Front proach*/

#define TFT_COLOR_DEPTH 16
#define TFT_BL_PORT IO_PORTX
#define TFT_BL_PIN  IO_PINX

#endif

/*********************
 *   DEVICE CONFIG
 *********************/

/*===================
 *   User interface
 *==================*/

/*----------
 *   LED
 *---------*/
#define USE_LED         1
#if USE_LED != 0
#define LED_LIFE        LED1
#define LED_ERR         LED2
#define LED_BUSY        LED3
#define LED1_PORT       IO_PORTX
#define LED1_PIN        IO_PINX
#define LED2_PORT       IO_PORTX
#define LED2_PIN        IO_PINX
#define LED3_PORT       IO_PORTX
#define LED3_PIN        IO_PINX
#define LED4_PORT       IO_PORTX
#define LED4_PIN        IO_PINX
#define LED5_PORT       IO_PORTX
#define LED5_PIN        IO_PINX
#define LED6_PORT       IO_PORTX
#define LED6_PIN        IO_PINX
#define LED7_PORT       IO_PORTX
#define LED7_PIN        IO_PINX
#define LED8_PORT       IO_PORTX
#define LED8_PIN        IO_PINX
#endif /*USE_LED*/

/*------------- 
 *   Buzzer
 *------------*/
#define USE_BUZZER  0
#if USE_BUZZER != 0
#define BUZZER_PORT IO_PORTX
#define BUZZER_PIN  IO_PINX
#define BUZZER_INV  0
#define BUZZER_BEEP_ON_TIME 200
#define BUZZER_BEEP_OFF_TIME 100
#define BUZZER_EXT_DRIVE    0  /*Not supported yet*/
#endif

/*-----------
 *    Log
 *----------*/
#define USE_LOG     0
#if USE_LOG     !=  0
/*  0: none, 1: err, 2: err + warn, 3: err + warn + msg,  will be logged*/
#define LOG_LEVEL   3
#define LOG_SYM_MSG  ""
#define LOG_SYM_WARN ">"
#define LOG_SYM_ERR  "#"
#define LOG_LENGTH_MAX  512

#define LOG_FILE        ""
#define LOG_USE_PRINTF  0
#define LOG_USE_SERIAL  0
#define LOG_SERIAL_DRV  HW_SERIAL1
#define LOG_SERIAL_BAUD 115200
#endif /*USE_LOG*/

/*===============================
 *   Analog/Digital Converters
 *=============================*/

/*---------------------------------------
 *  HX711 (24 bit ADC)
 *---------------------------------------*/
#define USE_HX711     1
#if USE_HX711 != 0
#define HX711_SDO_PORT    IO_PORTD
#define HX711_SDO_PIN     IO_PIN1
#define HX711_SCK_PORT    IO_PORTA
#define HX711_SCK_PIN     IO_PIN14
#endif

/*=======================
 *   Display controllers
 *======================*/

/*----------------
 *    SSD1963
 *--------------*/
#define USE_SSD1963   0
#if USE_SSD1963 != 0
#define SSD1963_PAR_CS    PAR_CSX
#define SSD1963_RST_PORT  IO_PORTX
#define SSD1963_RST_PIN   IO_PINX
#define SSD1963_BL_PORT   IO_PORTX
#define SSD1963_BL_PIN    IO_PINX
/*Display settings*/
#define SSD1963_HDP     479
#define SSD1963_HT      531
#define SSD1963_HPS     43
#define SSD1963_LPS     8
#define SSD1963_HPW     10
#define SSD1963_VDP     271
#define SSD1963_VT      288
#define SSD1963_VPS     12
#define SSD1963_FPS     4
#define SSD1963_VPW     10
#define SSD1963_HS_NEG  0   /*Negative hsync*/
#define SSD1963_VS_NEG  0   /*Negative vsync*/
#define SSD1963_ORI     0   /*0, 90, 180, 270*/
#define SSD1963_COLOR_DEPTH 16
#endif


/*----------------
 *    R61581
 *--------------*/
#define USE_R61581   0
#if USE_R61581 != 0
#define R61581_PAR_CS    PAR_CSX
#define R61581_RS_PORT   IO_PORTX
#define R61581_RS_PIN    IO_PINX
#define R61581_RST_PORT  IO_PORTX
#define R61581_RST_PIN   IO_PINX
#define R61581_RST_PORT  IO_PORTX
#define R61581_RST_PIN   IO_PINX
#define R61581_BL_PORT   IO_PORTX
#define R61581_BL_PIN    IO_PINX
/*Display settings*/
#define R61581_HOR_RES     480
#define R61581_VER_RES     320
#define R61581_HDP     479
#define R61581_HT      531
#define R61581_HPS     43
#define R61581_LPS     8
#define R61581_HPW     10
#define R61581_VDP     271
#define R61581_VT      319
#define R61581_VPS     12
#define R61581_FPS     4
#define R61581_VPW     10
#define R61581_HS_NEG  0   /*Negative hsync*/
#define R61581_VS_NEG  0   /*Negative vsync*/
#define R61581_ORI     180   /*0, 90, 180, 270*/
#define R61581_COLOR_DEPTH 16
#endif

/*------------------------------
 *  ST7565 (Monochrome, low res.)
 *-----------------------------*/
#define USE_ST7565  0
#if USE_ST7565 != 0
#define ST7565_DRV      HW_SPIX_CSX
#define ST7565_RST_PORT IO_PORTX
#define ST7565_RST_PIN  IO_PINX
#define ST7565_RS_PORT  IO_PORTX
#define ST7565_RS_PIN   IO_PINX
#endif  /*USE_ST7565*/

/*------------------------------
 *  Remote display (via serial)
 *-----------------------------*/
#define USE_RDISP  0
#if USE_RDISP != 0
#define RDISP_DRV		HW_SERIALX
#define RDISP_BAUD		115200
#define RDISP_HOR_RES	320
#define RDISP_VER_RES	240
#endif  /*USE_RDISP*

/*====================
 * Display input
 *===================*/

/*--------------
 *    XPT2046
 *--------------*/
#define USE_XPT2046     0
#if USE_XPT2046 != 0
#define XPT2046_SPI_DRV     HW_SPIX_CSX
#define XPT2046_IRQ_PORT    IO_PORTX
#define XPT2046_IRQ_PIN     IO_PINX
#define XPT2046_HOR_RES     480
#define XPT2046_VER_RES     320
#define XPT2046_X_MIN       200
#define XPT2046_Y_MIN       200 
#define XPT2046_X_MAX       3800
#define XPT2046_Y_MAX       3800
#define XPT2046_AVG         4 
#define XPT2046_INV         0 
#endif

/*-----------------
 *    FT5406EE8
 *-----------------*/
#define USE_FT5406EE8    0
#if USE_FT5406EE8 != 0
#define FT540EE8_I2C_DRV    HW_I2C1
#endif

/*-------------------------------
 *    Mouse or touchpad on PC
 *------------------------------*/
#define USE_MOUSE 0
#if USE_MOUSE != 0
/*No settings*/
#endif

/*====================
 *  External memory
 *===================*/

/*------------
 *  SD card
 *------------*/
#define USE_SDCARD     0
#if USE_SDCARD !=0
#define SDCARD_SPI_DRV     HW_SPIX_CSX
#endif

/*====================
 *  	Sensors
 *===================*/

/*------------------------------
 *  ICM20602 (acc, gyro, temp)
 *-----------------------------*/
#define USE_ICM20602     0
#if USE_ICM20602 != 0
#define ICM20602_SPI_DRV     HW_SPIX_CSX
#endif

*---------------------------------------
 *  HCSR04 (Ultrasonic proximity module)
 *---------------------------------------*/
#define USE_HCSR04     1
#if USE_HCSR04 != 0
#define HCSR04_TRIG_PORT    IO_PORTG
#define HCSR04_TRIG_PIN     IO_PIN6
#define HCSR04_ECHO_PORT    IO_PORTD
#define HCSR04_ECHO_PIN     IO_PIN15
#endif
        
/*===============
 * WiFi
 *==============*/

/*------------
 *  ESP8266
 *------------*/
#define USE_ESP8266     0
#if USE_ESP8266 != 0
#define ESP8266_DRV         HW_SERIALX
#define ESP8266_BUF_SIZE    256
#define ESP8266_LOG_REC_LINES   1
#endif

/*===============
 * GSM
 *==============*/

/*------------
 *  SIM5320
 *------------*/
#define USE_SIM5320     0
#if USE_SIM5320 != 0
#define SIM5320_DRV         HW_SERIALX
#define SIM5320_BUF_SIZE    256
#define SIM5320_LOG_REC_LINES   1
#endif

/*===============
 * IO EXPANDER
 *==============*/

/*------------
 *  MCP23008
 *------------*/
#define USE_MCP23008     0
#if USE_MCP23008 != 0
#define MCP23008_DRV     HW_I2CX
#define MCP23008_ADR     0b0100xxx
#endif

/*------------
 *  PCF8574
 *------------*/
#define USE_PCF8574     0
#if USE_PCF8574 != 0
#define PCF8574_DRV     HW_I2CX
#define PCF8574_ADR     0b0100xxx
#endif

#endif  /*HW_CONF_H*/

#endif /* Remove this line to enable the content */
