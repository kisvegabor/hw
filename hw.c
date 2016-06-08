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
#include "per/pwm.h"
#include "per/serial.h"
#include "per/par.h"
#include "per/spi.h"
#include "per/spim.h"
#include "per/i2c.h"
#include "per/adc.h"
#include "per/tft.h"
#include "dev/ui/led.h"
#include "dev/ui/buzzer.h"
#include "dev/ui/log.h"
#include "dev/dispc/SSD1963.h"
#include "dev/dispc/R61581.h"
#include "dev/dispc/ST7565.h"
#include "dev/tp/XPT2046.h"
#include "dev/tp/FT5406EE8.h"
#include "dev/tp/mouse.h"
#include "dev/adc/ads1256.h"
#include "dev/adc/ads1256_m.h"
#include "dev/bt/btm41x.h"
#include "dev/data_store/sdcard.h"
#include "dev/sens/isl29023.h"

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
    
#if USE_PWM != 0
    pwm_init();
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

#if USE_SPIM != 0
    spim_init();
#endif

#if USE_I2C != 0
    i2c_init();
#endif

#if USE_ADC != 0
    adc_init();
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

#if USE_SDCARD != 0
    sdcard_init();
#endif
    
#if USE_SSD1963 != 0
    ssd1963_init();
#endif
    
#if USE_R61581 != 0
    r61581_init();
#endif
    
#if USE_BTM41X != 0
    btm41x_init();
#endif
    
#if USE_ST7565 != 0
    st7565_init();
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
    
#if USE_ADS1256 != 0
    ads1256_init();
#endif
    
#if USE_ADS1256_M != 0
    ads1256_m_init();
#endif
    
#if USE_ISL29023 != 0
    isl29023_init();
#endif
    
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
