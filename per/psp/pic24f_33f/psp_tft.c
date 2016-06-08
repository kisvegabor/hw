/**
 * @file tft.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "../../../../hw_conf.h"
#if PSP_PIC24F_33F != 0 && USE_TFT != 0

#include <xc.h>
#include "../psp_tft.h"
#include "../../io.h"
#include "../../tft.h"

/*********************
 *      DEFINES
 *********************/

#define HSST_        0
#define DPWT_        TFT_HOR_RES + TFT_HSL + TFT_HBP + TFT_HFP
#define HENST_       HSST_ + TFT_HSL + TFT_HBP
#define ACTPIX_      HENST_

#define VSST_       0
#define DPHT_       TFT_VER_RES + TFT_VSL + TFT_VBP + TFT_VFP
#define VENST_      VSST_ + TFT_VSL + TFT_VBP
#define ACTLINE_    VENST_

// color depth options
#if TFT_COLOR_DEPTH == 8
#define COLOR_MODE  3
#elif TFT_COLOR_DEPTH == 16
#define COLOR_MODE  4
#endif

#define ADDRESS_RCCGPU  6

#define CMD_SOURCE_ADDR_OFFSET  2
#define CMD_DEST_ADDR_OFFSET    3
#define CMD_RECTANGLE_SIZE      4
#define CMD_RCC_COLOR           6
#define CMD_START               7

#define OP_COPY_SOLID_FILL      0
#define OP_COPY_RAST_OPER       1
#define OP_COPY_ROP_TRANSP      6

#define OP_ROP_RASTER_BLACK    0x00       //black will be the destination pixel
#define OP_ROP_RASTER_WHITE    0x0F
#define OP_ROP_RASTER_SOURCE   0x0C     //the source pixel will be the destination

#define OP_SOURCE_TYPE_DISC     0       //Source address is discontinuous type.
#define OP_SOURCE_TYPE_CONT     1       //Source address is continuous type.

#define OP_DEST_TYPE_DISC       0       //Source address is discontinuous type.
#define OP_DEST_TYPE_CONT       1       //Source address is continuous type.


/**********************
 *      TYPEDEFS
 **********************/

typedef union
{
    struct
    {
        uint16_t offset :16;
    };
    uint16_t value;
}GCMD_destaddrL_t;

typedef union
{
    struct
    {
        uint16_t offset :8;
        uint16_t cmd :4;
        uint16_t addr:4;
    };
    uint16_t value;
}GCMD_destaddrH_t;

typedef union
{
    struct
    {
        uint16_t offset :16;
    };
    uint16_t value;
}GCMD_srceaddrL_t;

typedef union
{
    struct
    {
        uint16_t offset :8;
        uint16_t cmd :4;
        uint16_t addr:4;
    };
    uint16_t value;
}GCMD_srcaddrH_t;


typedef union
{
    struct
    {
        uint16_t height :11;
        uint16_t res :1;
        uint16_t widthl :4;
    };
    uint16_t value;
}GCMD_rectsizeL_t;

typedef union
{
    struct
    {
        uint16_t widthh :7;
        uint16_t res :1;
        uint16_t cmd :4;
        uint16_t addr :4;
    };
    uint16_t value;
}GCMD_rectsizeH_t;

typedef union
{
    struct
    {
        uint16_t RES1 :1;
        uint16_t ST :1;
        uint16_t DT :1;
        uint16_t ROP :4;
        uint16_t OPER :3;
        uint16_t RES2 :5;
    };
    uint16_t value;
}GCMD_startcopyL_t;

typedef union
{
    struct
    {
        uint16_t RES :8;
        uint16_t CMD :4;
        uint16_t ADR :4;
                
    };
    uint16_t value;
}GCMD_startcopyH_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
 static uint8_t myframe_buffer[(uint32_t)((uint32_t)TFT_HOR_RES * TFT_VER_RES * TFT_COLOR_DEPTH) / 8UL] 
__attribute__((space(eds),noload)); 
__eds__ static uint8_t * p_myframe_buff; 

static int32_t last_x1;
static int32_t last_y1;
static int32_t last_x2;
static int32_t last_y2;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * 
 */
hw_res_t psp_tft_init(void)
{
    hw_res_t res = HW_RES_OK;
    
    CLKDIVbits.G1CLKSEL = 1;             //1 = Use the 96 MHz clock as a graphics controller module clock
    //CLKDIV2bits.GCLKDIV = 111;              //111 : divide by 48 --> 1 or 2 MHz
    CLKDIV2bits.GCLKDIV = 56;               //GCLKDIV = 56 -->6.40 MHz (divide by 15)

    p_myframe_buff = myframe_buffer;        // pointer points to the frame buffer base address
    uint32_t fbuff_address = (uint32_t) p_myframe_buff;           
    uint16_t fbuff_address_low = (uint16_t) (fbuff_address & 0xFFFF);            //low two byte (of 3 byte)
     
    G1CON1bits.GCMDWMK = 12;             // If the number of commands present in fifo
    G1CON1bits.PUBPP = COLOR_MODE;           //GPU bits-per-pixel Setting bits
    G1CON2bits.DPBPP = COLOR_MODE;           //Display bits-per-pixel Setting bits
    G1CON2bits.DPMODE = 1;              //TFT type
    G1CON2bits.DPSTGER = 0;             // Delays of the display data are staggered in groups:
    G1CON3bits.DPPINOE = 0;             //Enable display output pads késobb
    G1CON3bits.DPCLKPOL = 0;            //0 = Display latches data on negative edge of GCLK -
    G1CON3bits.DPENPOL = 1;             //Active-high (GEN) +
    G1CON3bits.DPVSPOL = 0;             //Active-low (VSYNC) +
    G1CON3bits.DPHSPOL = 0;             //Active-low (HSYNC) +
    G1CON3bits.DPPWROE = 0;             //GPWR port disabled (pin can be used as ordinary I/O) -
    G1CON3bits.DPPOWER = 0;         
    G1CON3bits.DPENOE = 1;              //GEN port enabled +
    G1CON3bits.DPVSOE = 1;              //VSYNC port enabled +
    G1CON3bits.DPHSOE = 1;              //HSYNC port enabled +
    G1PUW = TFT_HOR_RES;                //PUW<10:0>: GPU Work Area Width bits (in pixels)
    G1PUH = TFT_VER_RES;                //PUH<10:0>: GPU Work Area Height bits (in pixels)
    G1DPW = TFT_HOR_RES;                //DPW<10:0>: Display Frame Width bits (in pixels)
    G1DPH = TFT_VER_RES;                //DPH<10:0>: Display Frame Height bits (in pixels)
    G1DPWT = DPWT_;                     //DPWT<10:0>: Display Total Width bits (in pixels)
    G1DPHT = DPHT_;                     //DPHT<10:0>: Display Total Height bits (in pixels)
    G1ACTDAbits.ACTLINE = ACTLINE_;     //Number of Lines Before the First Active (displayed) Line bits -
    G1ACTDAbits.ACTPIX = ACTPIX_;       //Number of Pixels Before the First Active (displayed) Pixel bits (in DISPCLKs) -
    G1HSYNCbits.HSLEN = TFT_HSL;         //HSYNC Pulse-Width Configuration bits (in DISPCLKs)
    G1HSYNCbits.HSST = HSST_;           //HSYNC Start Delay Configuration bits (in DISPCLKs)
    G1VSYNCbits.VSLEN = TFT_VSL;         //VSYNC Pulse-Width Configuration bits (in lines)
    G1VSYNCbits.VSST = VSST_;           //VSYNC Start Delay Configuration bits (in lines)
    G1DBLCONbits.VENST = VENST_;        // VENST = ACTLINE
    G1DBLCONbits.HENST = HENST_;        //HENST = ACTPIX
    G1CLUTbits.CLUTEN = 0;              //1 = Color look-up table enabled
    G1DBENbits.GDBEN =  0xFFFF;         //1 = Corresponding Display Data (GD<x>) pin is enabled
    G1CON3bits.DPPINOE =1;
    G1CON3bits.DPPOWER = 1;
    G1CON2bits.DPTEST = 0;

    //next lines define the features of the frame buffer

    G1W1ADRL = fbuff_address_low;           //GPU Work Area 1 Start Address Low bits  (2byte)
    G1W1ADRH = 0;                           //GPU Work Area 1 Start Address High bits (1byte)
    G1W2ADRL = fbuff_address_low;           //GPU Work Area 2 Start Address Low bits  (2byte)
    G1W2ADRH = 0;
    G1PUW = TFT_HOR_RES;                           //GPU Work Area Width Register (in pixels).
    G1PUH = TFT_VER_RES;                           //GPU Work Area Height Register (in pixels).
    
    G1DPADRL =  fbuff_address_low;          //Display Buffer Start Address Register Low
    G1DPADRH =  0;                          //Display Buffer Start Address Register High
    G1DPW = TFT_HOR_RES;                    //Display Buffer Width Register (in pixels).
    G1DPH = TFT_VER_RES;                    //Display Buffer High Register (in pixels).

    
    G1CON1bits.G1EN = 1;                //enable display modul

    
    io_set_pin_dir(TFT_BL_PORT, TFT_BL_PIN, IO_DIR_OUT);
    io_set_pin(TFT_BL_PORT, TFT_BL_PIN, 0);
    
    return res;
}

/**
 * Mark out an area on the TFT
 * @param x1 left coordinate
 * @param y1 top coordinate
 * @param x2 right coordinate
 * @param y2 bottom coordinate
 */
void psp_tft_set_area(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
	last_x1 = x1;
	last_y1 = y1;
	last_x2 = x2;
	last_y2 = y2;
    
    if(last_x2 < TFT_HOR_RES) last_x2 ++;
    if(last_y2 < TFT_VER_RES) last_y2 ++;
}

/**
 * Fill out the marked area with a color
 * @param color fill color
 */
void psp_tft_fill(color_t color)
{
    
    uint32_t color_conv;
#if COLOR_DEPTH == TFT_COLOR_DEPTH
    color_conv = color.full;  
#elif TFT_COLOR_DEPTH == 4
    color_conv = color_to4(color);
#elif TFT_COLOR_DEPTH == 8
    color_conv = color_to8(color);
#elif TFT_COLOR_DEPTH == 16
    color_conv = color_to16(color);
#elif TFT_COLOR_DEPTH == 24
    color_conv = color_to24(color);
#endif   
    
    while((G1STATbits.CMDLV)== 0); //while the command number in CMDFIFO is more then GCMDWMK. (~fifo full)
    
    //Set color value to the desired color (RCC_COLOR).
    G1CMDL = color_conv;                          // this is the white color
    G1CMDH =  ( ADDRESS_RCCGPU << (28-16) ) | ( CMD_RCC_COLOR << (24-16) );
    
    //Set the pixel offset destination address; (set the left upper corner of the square))
    GCMD_destaddrL_t addr_l;
    GCMD_destaddrH_t addr_h;
    uint32_t offset = (uint32_t)last_x1 + ( (uint32_t)last_y1 * TFT_HOR_RES );
    addr_l.offset = offset & 0xFFFF;
    addr_h.offset=((uint32_t)offset >> 16) & 0xFF;
    addr_h.cmd = CMD_DEST_ADDR_OFFSET;
    addr_h.addr = ADDRESS_RCCGPU;
    G1CMDL = addr_l.value;
    G1CMDH = addr_h.value;        


    //Set width = width of the rectangle and height = height of the rectangle (RCCRECTSIZE)
    uint16_t actual_width = last_x2 - last_x1; 
    GCMD_rectsizeL_t g_low;
    GCMD_rectsizeH_t g_high;   
    g_low.height = (last_y2 - last_y1) & 0x07FF;
    g_low.widthl = (actual_width) & 0x0F;   
    g_high.widthh = (actual_width >> 4) & 0x7F;
    g_high.addr = ADDRESS_RCCGPU;
    g_high.cmd = CMD_RECTANGLE_SIZE;
    G1CMDL = g_low.value;
    G1CMDH = g_high.value;


    //Set start processing command 
    GCMD_startcopyL_t start_l;
    GCMD_startcopyH_t start_h;
    start_l.ST = OP_SOURCE_TYPE_DISC;
    start_l.DT = OP_DEST_TYPE_DISC;
    start_l.ROP = OP_ROP_RASTER_SOURCE;
    start_l.OPER = OP_COPY_SOLID_FILL;
    start_h.CMD = CMD_START;
    start_h.ADR = ADDRESS_RCCGPU;
    G1CMDL = start_l.value;
    G1CMDH = start_h.value;   
}

/**
 * Put a color map to the marked area
 * @param color_p an array of colors
 */
void psp_tft_map(color_t * map_p)
{
    while((G1STATbits.CMDLV)== 0); //while the command num in CMDFIFO is more then GCMDWMK. (~fifo full)
    
    //set the GPU WOOR AREA 1 to the address of the address of the array to be copied.   
    uint32_t src_address =  (unsigned long)(void __eds__ *) map_p;            //low two byte (of 3 byte)    
    uint32_t addrlow = (uint16_t) (src_address & 0xFFFF); 
    G1W1ADRL = addrlow;
    uint32_t addrhigh = (uint16_t)(src_address >> 16 & 0xFFFF); 
    G1W1ADRH = addrhigh;
  
    //Set the pixel offset source address
    GCMD_srceaddrL_t source_addr_l;
    GCMD_srcaddrH_t source_addr_h;
    //offset is 0 because the fist byte to be copied is the address of the array
    source_addr_l.offset = 0;                 //(u16) offset & 0xFFFF;
    source_addr_h.offset= 0;                       //(u16) (offset & 0xFF0000 >> 16);
    source_addr_h.cmd = CMD_SOURCE_ADDR_OFFSET;
    source_addr_h.addr = ADDRESS_RCCGPU;
    G1CMDL = source_addr_l.value;
    G1CMDH = source_addr_h.value;      
    while((G1STATbits.CMDLV)== 0); //while the command num in CMDFIFO is more then GCMDWMK. (~fifo full)  

    //Set the pixel offset destination address
    GCMD_destaddrL_t dest_addr_l;
    GCMD_destaddrH_t dest_addr_h;
    uint32_t offset = (uint32_t)last_x1 + ((uint32_t) last_y1 * TFT_HOR_RES );
    dest_addr_l.offset = offset & 0xFFFF;
    dest_addr_h.offset= ((uint32_t)offset >> 16) & 0xFF;
    dest_addr_h.cmd = CMD_DEST_ADDR_OFFSET;
    dest_addr_h.addr = ADDRESS_RCCGPU;
    G1CMDL = dest_addr_l.value;
    G1CMDH = dest_addr_h.value; 
    
    while((G1STATbits.CMDLV)== 0);
    //Set width = width of the rectangle and height = 1
    GCMD_rectsizeL_t g_low;
    GCMD_rectsizeH_t g_high;     
    int16_t w = last_x2 - last_x1;
    g_low.height = last_y2 - last_y1; //ST and DT settings will not matter here since the operation will be performed with height = 1
    g_low.widthl = (uint16_t)w & 0x0F;   
    g_high.widthh = (uint16_t)(w >> 4) & 0x7F;
    g_high.addr = ADDRESS_RCCGPU;
    g_high.cmd = CMD_RECTANGLE_SIZE;
    G1CMDL = g_low.value;
    G1CMDH = g_high.value;

    //Set start processing command 
    GCMD_startcopyL_t start_l;
    GCMD_startcopyH_t start_h;
    start_l.ST = OP_SOURCE_TYPE_CONT;
    start_l.DT = OP_DEST_TYPE_DISC;
    start_l.ROP = OP_ROP_RASTER_SOURCE;
    start_l.OPER = OP_COPY_RAST_OPER;
    start_h.CMD = CMD_START;
    start_h.ADR = ADDRESS_RCCGPU;
    G1CMDL = start_l.value;
    G1CMDH = start_h.value;    
}



/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
