/**
 * @file psp_i2c.c
 * 
 * TODO Only support master mode
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_I2C != 0 && PSP_PIC24F_33F != 0

#include <xc.h>
#include <stddef.h>
#include "../psp_i2c.h"

/*********************
 *      DEFINES
 *********************/

/*For compability check how I2CCON defined*/
#ifdef I2C1CON1
#define MY_I2CXCON(x) I2C ## x ##CON1
#else
#define MY_I2CXCON(x) I2C##x##CON
#endif

/*Create the I2CXCONBITS type as well*/
#define MY_I2CXCON_T__(reg) reg##BITS
#define MY_I2CXCON_T(reg) MY_I2CXCON_T__(reg)

#define PSP_I2C_BRG_MAX     511 

/**********************
 *      TYPEDEFS
 **********************/
typedef struct
{
    /*Registers*/
    volatile MY_I2CXCON_T(MY_I2CXCON(1)) * I2CxCON;
    volatile I2C1STATBITS * I2CxSTAT;
    volatile unsigned int * I2CxBRG;
    volatile unsigned int * I2CxTRN;
    volatile unsigned int * I2CxRCV;
    
    /*Settings*/
    uint32_t baud;
}m_dsc_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static hw_res_t psp_i2c_idle(i2c_t id);

/**********************
 *  STATIC VARIABLES
 **********************/
static m_dsc_t m_dsc[] = 
{
        /*CON*/                                                  /*STAT*/        /*BRG*/    /*TRN*/   /*RCV*/   /*baud*/
#if I2C1_BAUD != 0
   {(MY_I2CXCON_T(MY_I2CXCON(1)) * ) &MY_I2CXCON(1), (I2C1STATBITS *) &I2C1STAT, &I2C1BRG, &I2C1TRN, &I2C1RCV, I2C1_BAUD},   
#else
   {NULL,                       NULL,                      NULL,    NULL,       NULL,    0},
#endif
#if I2C2_BAUD != 0
   {(MY_I2CXCON_T(MY_I2CXCON(1)) * ) &MY_I2CXCON(2), (I2C1STATBITS *) &I2C2STAT, &I2C2BRG, &I2C2TRN, &I2C2RCV, I2C2_BAUD}, 
#else
   {NULL,                       NULL,                      NULL,    NULL,       NULL,    0},
#endif   
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the i2c periphery. 
 */
void psp_i2c_init(void)
{
    i2c_t i;
    
    for(i = HW_I2C1; i < HW_I2C_NUM; i++) {
        if(m_dsc[i].I2CxCON  != NULL) {
            m_dsc[i].I2CxCON->I2CEN = 1;
            uint32_t baud = (uint32_t) CLOCK_PERIPH / m_dsc[i].baud;
            if(baud > 4) baud -= 2;
            else baud = 2;
            
            if(baud > PSP_I2C_BRG_MAX) baud = PSP_I2C_BRG_MAX;
            *m_dsc[i].I2CxBRG = baud;            
        }
    }
}


/**
 * Make a start condition
 * @param id id of an i2c (from i2c_t)
 * @return HW_RES_OK or any error from hw_res_t
 */
hw_res_t psp_i2c_start(i2c_t id)
{
    hw_res_t res = HW_RES_OK;
        
    /*If a register is NULL then the module is disabled*/
    if(m_dsc[id].I2CxCON != NULL) {
        m_dsc[id].I2CxCON->SEN = 1;          /* Set start condition */
        while (m_dsc[id].I2CxCON->SEN == 1); /* Wait till start condition is cleared */
        psp_i2c_idle(id);
    } else {
        res = HW_RES_DIS;
    }
    return res;
}

/**
 * Make a restart condition
 * @param id id of an i2c (from i2c_t)
 * @return HW_RES_OK or any error from hw_res_t
 */
hw_res_t psp_i2c_restart(i2c_t id)
{
    hw_res_t res = HW_RES_OK;
    
    /*If a register is NULL then the module is disabled*/
    if(m_dsc[id].I2CxCON != NULL) {
        m_dsc[id].I2CxCON->RSEN = 1;                       /* Set restart condition */
        while (m_dsc[id].I2CxCON->RSEN == 1);              /* Wait till restart condition is cleared */
        psp_i2c_idle(id);  
    } else {
        res = HW_RES_DIS;
    }
    
    return res;
}

/**
 * Make a stop condition
 * @param id id of an i2c (from i2c_t)
 * @return HW_RES_OK or any error from hw_res_t
 */
hw_res_t psp_i2c_stop(i2c_t id)
{
    hw_res_t res = HW_RES_OK;        

    /*If a register is NULL then the module is disabled*/
    if(m_dsc[id].I2CxCON != NULL) {
        m_dsc[id].I2CxCON->PEN = 1;                        /* Set stop condition */
        while (m_dsc[id].I2CxCON->PEN == 1);               /* Wait till stop condition is cleared*/
        psp_i2c_idle(id);
    } else {
        res = HW_RES_DIS;
    }

    return res;
}

/**
 * Write a byte to the i2c bus
 * @param id id of an i2c (from i2c_t)
 * @param data byte to write
 * @return HW_RES_OK or any error from hw_res_t
 */
hw_res_t psp_i2c_wr(i2c_t id, uint8_t data)
{
    hw_res_t res = HW_RES_OK;
    
    /*If a register is NULL then the module is disabled*/
    if(m_dsc[id].I2CxCON != NULL) {
        *(m_dsc[id].I2CxTRN) = data;             /* Write data into register */
        while (m_dsc[id].I2CxSTAT->TRSTAT == 1); /* Wait till transmit is in progress */
        psp_i2c_idle(id);   /* wait for bus idle */
        if(m_dsc[id].I2CxSTAT->ACKSTAT != 0) return HW_RES_NO_ACK;
    } else {
        res = HW_RES_DIS;
    }
    
    return res;
}

/**
 * Read a byte from the i2c bus
 * @param id id of an i2c (from i2c_t)
 * @param data pointer to a variable to store the read data
 * @param ack true: send ack, false: not sen ac
 * @return HW_RES_OK or any error from hw_res_t
 */
hw_res_t psp_i2c_rd(i2c_t id, uint8_t * data, bool ack)
{
    hw_res_t res = HW_RES_OK;
        
    /*If a register is NULL then the module is disabled*/
    if(m_dsc[id].I2CxCON != NULL) {
        psp_i2c_idle(id );               /* wait for bus idle */    
        m_dsc[id].I2CxCON->RCEN = 1;            /* enable receive */    
        while (m_dsc[id].I2CxCON->RCEN); /* wait for receive buffer full */    
        *data = *(m_dsc[id].I2CxRCV);    /* read the data */    

        /* send or not an ACK */
        if (ack != false) {        
           m_dsc[id].I2CxCON->ACKEN = 1;    /* send acknoledgement */
        } else {        
            m_dsc[id].I2CxCON->ACKEN = 0;   /* no acknoledgement */
        }        
    } else {
        res = HW_RES_DIS;
    }
  
    return res;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Wait until the i2c bus become idle
 * @param id id of an i2c (from i2c_t)
 * @return  HW_RES_OK or any error from hw_res_t
 */
static hw_res_t psp_i2c_idle(i2c_t id)
{
    hw_res_t res = HW_RES_OK;
    
    /* Wait until I2C Bus is Inactive */
    if(res == HW_RES_OK) {
        while(m_dsc[id].I2CxCON->SEN || m_dsc[id].I2CxCON->RSEN || 
              m_dsc[id].I2CxCON->PEN || m_dsc[id].I2CxCON->RCEN || 
              m_dsc[id].I2CxCON->ACKEN);
    }
    
    return res;
}

#endif
