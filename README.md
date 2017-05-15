# Periphery and device drivers
The repository contains some drivers for microcontroller peripheries like:
* GPIO
* UART
* SPI
* I2C
* TFT
* etc

And for devices like:
* Display controllers: e.g. SSD1963
* Touch pad controllers: e.g. XPT2046
* LED
* Buzzer
* etc

There are drivers for different microcontroller families and even for PC. You can easily change between families by modifying a define in *hw_conf.h*
because all drivers use the same API.

Currently mainly PIC microcontrollers and PC are supported:
* PIC24F/33E
* PIC32MX
* PIC32MZ
* PC

## Usage
1. Clone the repository into the root folder of your project: `https://github.com/littlevgl/hw.git`
2. In your IDE add the project **root folder as include path**
3. Copy *hw/hw_conf_templ.h* as **hw_conf.h** to the project root folder
4. Delete the first `#if 0` and the last `#endif` to enable the file
5. Choose the hardware family by: `#define PSP_...   1`
6. Enable/disable or configure the components
7. To initialize the library `#include hw/hw.h` and call `per_init()` and `dev_init()`

## Remark
It is not the goal of this library to create drivers for all peripheries and functions. 
When I use a new microcontroller and a new periphery I will add it. 

If you are using the library in your project pleas write your drivers according to the existing ones and share them! :)
