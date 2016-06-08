/**
 * @file device.h
 * 
 */

#ifndef DEVICE_H
#define DEVICE_H

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/
#define    HW_INT_PRIO_OFF      0
#define    HW_INT_PRIO_LOWEST   1
#define    HW_INT_PRIO_LOW      2
#define    HW_INT_PRIO_MID      3
#define    HW_INT_PRIO_HIGH     4
#define    HW_INT_PRIO_HIGHEST  5


/**********************
 *      TYPEDEFS
 **********************/
typedef enum
{
    /*No error*/
    HW_RES_OK = 0,

    /*Init time errors*/
    HW_RES_DIS,       /*Disabled by configuration settings (hw_conf.h)*/
    HW_RES_NOT_EX,    /*Module id shows a not existing module*/
    HW_RES_INV_PARAM, /*Invalid parameters*/

    /*"Usage" time errors*/
    HW_RES_NOT_RDY,   /*Not ready for this operation*/
    HW_RES_FULL,      /*Buffer is or become full*/
    HW_RES_EMPTY,     /*Buffer is or become empty */
    HW_RES_NO_ACK,    /*No acknowledgment*/
    HW_RES_TOUT,      /*Timeout*/
}hw_res_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void per_init(void);
void dev_init(void);

/**********************
 *      MACROS
 **********************/

#endif
