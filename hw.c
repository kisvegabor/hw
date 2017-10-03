/**
 * @file device.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw/hw.h"
#include "per/io.h"
#include "per/tmr.h"
#include "per/tick.h"
#include "per/serial.h"
#include "per/par.h"
#include "per/spi.h"
#include "per/i2c.h"
#include "per/tft.h"
#include "dev/ui/led.h"
#include "dev/ui/buzzer.h"
#include "dev/ui/log.h"
#include "dev/adc/hx711.h"
#include "dev/dispc/SSD1963.h"
#include "dev/dispc/R61581.h"
#include "dev/dispc/ST7565.h"
#include "dev/dispc/rdisp.h"
#include "dev/tp/XPT2046.h"
#include "dev/tp/FT5406EE8.h"
#include "dev/tp/mouse.h"
#include "dev/ext_mem/sdcard.h"
#include "dev/wifi/esp8266.h"
#include "dev/gsm/sim5320.h"
#include "dev/sens/icm20602.h"
#include "dev/sens/hcsr04.h"
#include "dev/io_exp/mcp23008.h"
#include "dev/io_exp/pcf8574.h"

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

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void per_init(void)
{
    
#if USE_IO != 0
    io_init();
#endif

#if USE_TMR != 0
    tmr_init();
#endif

#if USE_TICK != 0
    tick_init();
#endif
    
#if USE_SERIAL != 0
    serial_init();
#endif

#if USE_PARALLEL != 0
    par_init();
#endif

#if USE_SPI != 0
    spi_init();
#endif

#if USE_I2C != 0
    i2c_init();
#endif
    
#if USE_TFT != 0
    tft_init();
#endif
}

void dev_init(void)
{
    
#if USE_LED != 0
    led_init();
#endif
    
#if USE_BUZZER != 0
    buzzer_init();
#endif
    
#if USE_LOG != 0
    log_init();
#endif
    
#if USE_HX711
    hx711_init();
#endif

#if USE_SDCARD != 0
    sdcard_init();
#endif
    
#if USE_SSD1963 != 0
    ssd1963_init();
#endif
    
#if USE_R61581 != 0
    r61581_init();
#endif
    
#if USE_ST7565 != 0
    st7565_init();
#endif
    
#if USE_RDISP != 0
    rdisp_init();
#endif

#if USE_XPT2046 != 0
    xpt2046_init();
#endif
    
#if USE_FT5406EE8 != 0
    ft5406ee8_init();
#endif

#if USE_MOUSE != 0
    mouse_init();
#endif
    
#if USE_ESP8266 != 0
    esp8266_init();
#endif
    
#if USE_SIM5320 != 0
    sim5320_init();
#endif

#if USE_ICM20602 != 0
    icm20602_init();
#endif
    
#if USE_HCSR04
    hcsr04_init();
#endif
    
#if USE_MCP23008 != 0
    mcp23008_init();
#endif
    
#if USE_PCF8574 != 0
    pcf8574_init();
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
