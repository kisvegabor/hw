/**
 * @file sdcard.c
 * 
 */

/*------------------------------------------------------------------------/
/  MMCv3/SDv1/SDv2 (in SPI mode) control module
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2010, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/
//Edited by A. Morrison to function on PIC32.


/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_SDCARD != 0

#include <stddef.h>
#include "sdcard.h"
#include "hw/per/tick.h"
#include "hw/per/spi.h"
#include "misc/fs/fat32/ff.h"

/*********************
 *      DEFINES
 *********************/
/* Definitions for MMC/SDC command */
#define CMD0   (0)			/* GO_IDLE_STATE */
#define CMD1   (1)			/* SEND_OP_COND */
#define ACMD41 (41|0x80)	/* SEND_OP_COND (SDC) */
#define CMD8   (8)			/* SEND_IF_COND */
#define CMD9   (9)			/* SEND_CSD */
#define CMD10  (10)			/* SEND_CID */
#define CMD12  (12)			/* STOP_TRANSMISSION */
#define ACMD13 (13|0x80)	/* SD_STATUS (SDC) */
#define CMD16  (16)			/* SET_BLOCKLEN */
#define CMD17  (17)			/* READ_SINGLE_BLOCK */
#define CMD18  (18)			/* READ_MULTIPLE_BLOCK */
#define CMD23  (23)			/* SET_BLOCK_COUNT */
#define ACMD23 (23|0x80)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24  (24)			/* WRITE_BLOCK */
#define CMD25  (25)			/* WRITE_MULTIPLE_BLOCK */
#define CMD41  (41)			/* SEND_OP_COND (ACMD) */
#define CMD55  (55)			/* APP_CMD */
#define CMD58  (58)			/* READ_OCR */


/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static int xmit_datablock (const uint8_t *buff, uint8_t token);
static uint8_t send_cmd (uint8_t cmd, uint32_t arg);
static void power_on (void);
static void power_off (void);
static int rcvr_datablock (	uint8_t *buff, uint32_t btr);
static uint8_t wait_ready (void);
static void deselect (void);
static int select (void);

/**********************
 *  STATIC VARIABLES
 **********************/
static volatile DSTATUS Stat = STA_NOINIT;	/* Disk status */
static volatile uint32_t Timer1, Timer2;		/* 1000Hz decrement timer */
static uint32_t CardType;
static bool inited = false;
static FATFS Fatfs;

/**********************
 *      MACROS
 **********************/


/* Port Controls  (Platform dependent) */
#define CS_SETOUT() {}
#define CS_LOW()  spi_cs_en(SDCARD_SPI_DRV)	/* MMC CS = L */
#define CS_HIGH() spi_cs_dis(SDCARD_SPI_DRV)	/* MMC CS = H */

// Makes assumptions that sockwp and sockins are on the same port...
// Should probably remove sockport define and then go fix what used it to be general.
#define SOCKPORT	PORTG		/* Socket contact port */
#define SOCKWP	0 // disable write protect (1<<10)		/* Write protect switch (RB10) */
#define SOCKINS	0 // Pretend card is always inserted (1<<11)	/* Card detect switch (RB11) */

#define SDCARD_SPI_BAUD_SLOW    200000  /*Slow 200 kHz clock. Used during initialization*/
#define SDCARD_SPI_BAUD_FAST    SPI_BAUD_MAX /*Fast 10Mhz clock for data transfer*/

/**
 * Initialize the SD card
 * @return 
 */
void sdcard_init(void)
{
    /*Initialize the fat32 itself*/
    DSTATUS d_res;
    FRESULT f_res = 0xFF;
    
    tick_add_cb(disk_timerproc);
    
    
    d_res=disk_initialize(0);
    if(d_res == RES_OK) {
        f_res=f_mount(&Fatfs, "\0", 1);
        if(f_res == FR_OK) {
            inited = true;
        }
    }
}

bool sdcard_ready(void)
{
    return inited;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

	/* Physical drive nmuber (0) */
	/* Pointer to the data to be written */
	/* Start sector number (LBA) */
	/* Sector count (1..255) */
DRESULT disk_write ( uint8_t drv, const uint8_t *buff, uint32_t sector, uint8_t count )
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	if (Stat & STA_PROTECT) return RES_WRPRT;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */

	if (count == 1) {		/* Single block write */
		if ((send_cmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
			&& xmit_datablock(buff, 0xFE))
			count = 0;
	} else {				/* Multiple block write */
		if (CardType & CT_SDC) send_cmd(ACMD23, count);
		if (send_cmd(CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK */
			do {
				if (!xmit_datablock(buff, 0xFC)) break;
				buff += 512;
			} while (--count);
			if (!xmit_datablock(0, 0xFD))	/* STOP_TRAN token */
				count = 1;
		}
	}
	deselect();

	return count ? RES_ERROR : RES_OK;
}




/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/
/* Physical drive nmuber (0) */
DSTATUS disk_initialize (uint8_t drv)
{
	uint8_t n, cmd, ty, ocr[4];

	if (drv) return STA_NOINIT;		/* Supports only single drive */
	if (Stat & STA_NODISK) return Stat;	/* No card in the socket */

	power_on();				/* Force socket power on */
	spi_set_baud(SDCARD_SPI_DRV, SDCARD_SPI_BAUD_SLOW);
    
    spi_xchg(SDCARD_SPI_DRV, NULL, NULL, 10);   /* 80 dummy clocks */
    
	ty = 0;
	if (send_cmd(CMD0, 0) == 1) {			/* Enter Idle state */
		Timer1 = 1000;				/* Initialization timeout of 1000 msec */
		if (send_cmd(CMD8, 0x1AA) == 1) {	/* SDv2? */
			for (n = 0; n < 4; n++) {
                spi_xchg(SDCARD_SPI_DRV, NULL, &ocr[n], 1); /* Get trailing return value of R7 resp */
            } 
            
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {				/* The card can work at vdd range of 2.7-3.6V */
				while (Timer1 && send_cmd(ACMD41, 0x40000000));	/* Wait for leaving idle state (ACMD41 with HCS bit) */
				if (Timer1 && send_cmd(CMD58, 0) == 0) {			/* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++) {
                        spi_xchg(SDCARD_SPI_DRV, NULL, &ocr[n], 1);
                    }
					ty = (ocr[0] & 0x40) ? CT_SD2|CT_BLOCK : CT_SD2;	/* SDv2 */
				}
			}
		} else {							/* SDv1 or MMCv3 */
			if (send_cmd(ACMD41, 0) <= 1) {
				ty = CT_SD1; cmd = ACMD41;	/* SDv1 */
			} else {
				ty = CT_MMC; cmd = CMD1;	/* MMCv3 */
			}
			while (Timer1 && send_cmd(cmd, 0));		/* Wait for leaving idle state */

            /* Set read/write block length to 512 */
			if (!Timer1 || send_cmd(CMD16, 512) != 0) {             
                ty = 0;
            }
		}
	}
        
	CardType = ty;
	deselect();

	if (ty)  {			/* Initialization succeded */
		Stat &= ~STA_NOINIT;	/* Clear STA_NOINIT */
		spi_set_baud(SDCARD_SPI_DRV, SDCARD_SPI_BAUD_FAST);
	} else  {			/* Initialization failed */
		power_off();
	}

	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/
/* Physical drive nmuber (0) */
DSTATUS disk_status (uint8_t drv)
{
	if (drv) return STA_NOINIT;		/* Supports only single drive */
	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
/* Physical drive nmuber (0) */
/* Pointer to the data buffer to store read data */
/* Sector count (1..255) */
/* Start sector number (LBA) */
DRESULT disk_read (uint8_t drv, uint8_t *buff, uint32_t sector, uint8_t count)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */

	if (count == 1) {		/* Single block read */
		if ((send_cmd(CMD17, sector) == 0)	/* READ_SINGLE_BLOCK */
			&& rcvr_datablock(buff, 512))
			count = 0;
	}
	else {				/* Multiple block read */
		if (send_cmd(CMD18, sector) == 0) {	/* READ_MULTIPLE_BLOCK */
			do {
				if (!rcvr_datablock(buff, 512)) break;
				buff += 512;
			} while (--count);
			send_cmd(CMD12, 0);				/* STOP_TRANSMISSION */
		}
	}
	deselect();

	return count ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
//
//DRESULT w (
//	uint8_t drv,				/* Physical drive nmuber (0) */
//	const uint8_t *buff,		/* Pointer to the data to be written */
//	uint32_t sector,			/* Start sector number (LBA) */
//	uint8_t count				/* Sector count (1..255) */
//)
//{
//	if (drv || !count) return RES_PARERR;
//	if (Stat & STA_NOINIT) return RES_NOTRDY;
//	if (Stat & STA_PROTECT) return RES_WRPRT;
//
//	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */
//
//	if (count == 1) {		/* Single block write */
//		if ((send_cmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
//			&& xmit_datablock(buff, 0xFE))
//			count = 0;
//	}
//	else {				/* Multiple block write */
//		if (CardType & CT_SDC) send_cmd(ACMD23, count);
//		if (send_cmd(CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK */
//			do {
//				if (!xmit_datablock(buff, 0xFC)) break;
//				buff += 512;
//			} while (--count);
//			if (!xmit_datablock(0, 0xFD))	/* STOP_TRAN token */
//				count = 1;
//		}
//	}
//	deselect();
//
//	return count ? RES_ERROR : RES_OK;
//}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
/* Physical drive nmuber (0) */
/* Control code */
/* Buffer to send/receive data block */
DRESULT disk_ioctl (uint8_t drv, uint8_t ctrl, void *buff)
{
	DRESULT res;
	uint8_t n, csd[16], *ptr = buff;
	uint32_t csize;


	if (drv) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	res = RES_ERROR;
	switch (ctrl) {
		case CTRL_SYNC :	/* Flush dirty buffer if present */
			if (select()) {
				deselect();
				res = RES_OK;
			}
			break;

		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (uint16_t) */
			if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
				if ((csd[0] >> 6) == 1) {	/* SDv2? */
					csize = csd[9] + ((uint16_t)csd[8] << 8) + 1;
					*(uint32_t*)buff = (uint32_t)csize << 10;
				} else {					/* SDv1 or MMCv2 */
					n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
					csize = (csd[8] >> 6) + ((uint16_t)csd[7] << 2) + ((uint16_t)(csd[6] & 3) << 10) + 1;
					*(uint32_t*)buff = (uint32_t)csize << (n - 9);
				}
				res = RES_OK;
			}
			break;

		case GET_SECTOR_SIZE :	/* Get sectors on the disk (uint16_t) */
			*(uint16_t*)buff = 512;
			res = RES_OK;
			break;

		case GET_BLOCK_SIZE :	/* Get erase block size in unit of sectors (uint32_t) */
			if (CardType & CT_SD2) {	/* SDv2? */
				if (send_cmd(ACMD13, 0) == 0) {		/* Read SD status */
                    spi_xchg(SDCARD_SPI_DRV, NULL, NULL, 1);
					if (rcvr_datablock(csd, 16)) {				/* Read partial block */
						for (n = 64 - 16; n; n--) {
                            spi_xchg(SDCARD_SPI_DRV, NULL, NULL, 1);    /* Purge trailing data */
                        }	
						*(uint32_t*)buff = 16UL << (csd[10] >> 4);
						res = RES_OK;
					}
				}
			} else {					/* SDv1 or MMCv3 */
				if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {	/* Read CSD */
					if (CardType & CT_SD1) {	/* SDv1 */
						*(uint32_t*)buff = (((csd[10] & 63) << 1) + ((uint16_t)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
					} else {					/* MMCv3 */
						*(uint32_t*)buff = ((uint16_t)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
					}
					res = RES_OK;
				}
			}
			break;

		case MMC_GET_TYPE :		/* Get card type flags (1 byte) */
			*ptr = CardType;
			res = RES_OK;
			break;

		case MMC_GET_CSD :	/* Receive CSD as a data block (16 bytes) */
			if ((send_cmd(CMD9, 0) == 0)	/* READ_CSD */
				&& rcvr_datablock(buff, 16))
				res = RES_OK;
			break;

		case MMC_GET_CID :	/* Receive CID as a data block (16 bytes) */
			if ((send_cmd(CMD10, 0) == 0)	/* READ_CID */
				&& rcvr_datablock(buff, 16))
				res = RES_OK;
			break;

		case MMC_GET_OCR :	/* Receive OCR as an R3 resp (4 bytes) */
			if (send_cmd(CMD58, 0) == 0) {	/* READ_OCR */
				for (n = 0; n < 4; n++) {
                    spi_xchg(SDCARD_SPI_DRV, NULL, (uint8_t*)buff+n, 1);
                }
				res = RES_OK;
			}
			break;

		case MMC_GET_SDSTAT :	/* Receive SD statsu as a data block (64 bytes) */
			if (send_cmd(ACMD13, 0) == 0) {	/* SD_STATUS */
                spi_xchg(SDCARD_SPI_DRV, NULL, NULL, 1);
				if (rcvr_datablock(buff, 64))
					res = RES_OK;
			}
			break;

		default:
			res = RES_PARERR;
	}

	deselect();

	return res;
}

/*-----------------------------------------------------------------------*/
/* Device Timer Interrupt Procedure  (Platform dependent)                */
/*-----------------------------------------------------------------------*/
/* This function must be called in period of 1ms                         */


void disk_timerproc (void)
{
	static uint16_t pv;
	uint16_t p;
	uint8_t s;
	uint32_t n;


	n = Timer1;						/* 1000Hz decrement timer */
	if (n) Timer1 = --n;
	n = Timer2;
	if (n) Timer2 = --n;

	p = pv;
	pv =(SOCKWP | SOCKINS);	/* Sample socket switch */

	if (p == pv) {						/* Have contacts stabled? */
		s = Stat;

		if (p & SOCKWP)					/* WP is H (write protected) */
			s |= STA_PROTECT;
		else							/* WP is L (write enabled) */
			s &= ~STA_PROTECT;

		if (p & SOCKINS)				/* INS = H (Socket empty) */
			s |= (STA_NODISK | STA_NOINIT);
		else							/* INS = L (Card inserted) */
			s &= ~STA_NODISK;

		Stat = s;
	}
}

/**********************
 *   STATIC FUNCTIONS
 **********************/


/*-----------------------------------------------------------------------*/
/* Send a data packet to MMC                                             */
/*-----------------------------------------------------------------------*/
/* 1:OK, 0:Failed */
/* 512 byte data block to be transmitted */
/* Data token */
static int xmit_datablock (const uint8_t *buff, uint8_t token)
{
	uint8_t resp;
	uint32_t bc = 512;

	if (wait_ready() != 0xFF) return 0;

    spi_xchg(SDCARD_SPI_DRV, &token, NULL, sizeof(token));  /* Xmit a token */
    
	if (token != 0xFD) {	/* Not StopTran token */


        do {						/* Xmit the 512 byte data block to the MMC */
            spi_xchg(SDCARD_SPI_DRV, buff, NULL, 2);
            buff += 2;
		} while (bc -= 2);


        spi_xchg(SDCARD_SPI_DRV, NULL, NULL, 2);  /* CRC (Dummy) Send 0xFF*/
  
        spi_xchg(SDCARD_SPI_DRV, NULL, &resp, 1); /* Receive a data response */
        
		if ((resp & 0x1F) != 0x05)	/* If not accepted, return with error */
			return 0;
	}

	return 1;
}
/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/
/* Command byte */
/* Argument */
static uint8_t send_cmd (uint8_t cmd, uint32_t arg)
{
	uint8_t n, res;

	if (cmd & 0x80) {	/* ACMD<n> is the command sequense of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1) return res;
	}

	/* Select the card and wait for ready */
	deselect();
	if (!select()) return 0xFF;

	/* Send command packet */
    uint8_t tmp = 0x40 | cmd;
    spi_xchg(SDCARD_SPI_DRV, &tmp, NULL, 1);    /* Start + Command index */
	tmp = (uint8_t)(arg >> 24);
    spi_xchg(SDCARD_SPI_DRV, &tmp, NULL, 1);    /* Argument[31..24] */
	tmp = (uint8_t)(arg >> 16);
    spi_xchg(SDCARD_SPI_DRV, &tmp, NULL, 1);    /* Argument[23..16] */
	tmp = (uint8_t)(arg >> 8);
    spi_xchg(SDCARD_SPI_DRV, &tmp, NULL, 1);    /* Argument[15..8] */
	tmp = (uint8_t)arg;
    spi_xchg(SDCARD_SPI_DRV, &tmp, NULL, 1);    /* Argument[7..0] */
    
    
	n = 0x01;						/* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95;		/* Valid CRC for CMD0(0) */
	if (cmd == CMD8) n = 0x87;		/* Valid CRC for CMD8(0x1AA) */
	spi_xchg(SDCARD_SPI_DRV, &n, NULL, 1);

	/* Receive command response */
	if (cmd == CMD12){
        spi_xchg(SDCARD_SPI_DRV, NULL, NULL, 1); /* Skip a stuff byte when stop reading */

    } 
	n = 10;							/* Wait for a valid response in timeout of 10 attempts */
	do {
         spi_xchg(SDCARD_SPI_DRV, NULL, &res, 1); 
    }
	while ((res & 0x80) && --n);

	return res;			/* Return with the response value */
}


/*-----------------------------------------------------------------------*/
/* Power Control  (Platform dependent)                                   */
/*-----------------------------------------------------------------------*/
/* When the target system does not support socket power control, there   */
/* is nothing to do in these functions and chk_power always returns 1.   */

static void power_on (void)
{
	// Setup CS as output
	CS_SETOUT();
    spi_set_baud(SDCARD_SPI_DRV, SDCARD_SPI_BAUD_SLOW);
}

static void power_off (void)
{
	select();			/* Wait for card ready */
	deselect();

	Stat |= STA_NOINIT;	/* Set STA_NOINIT */
}

/*-----------------------------------------------------------------------*/
/* Receive a data packet from MMC                                        */
/*-----------------------------------------------------------------------*/
/* 1:OK, 0:Failed */
/* Data buffer to store received data */
/* Byte count (must be multiple of 4) */
static int rcvr_datablock (	uint8_t *buff, uint32_t btr)
{
	uint8_t token;

	Timer1 = 100;
	do {				/* Wait for data packet in timeout of 100ms */
		spi_xchg(SDCARD_SPI_DRV, NULL, &token, 1);
	} while ((token == 0xFF) && Timer1);

	if(token != 0xFE) return 0;		/* If not valid data token, return with error */

    /* Receive the data block into buffer */
	do {
        spi_xchg(SDCARD_SPI_DRV, NULL, buff, 4);
        buff += 4;
	} while (btr -= 4);

	spi_xchg(SDCARD_SPI_DRV, NULL, NULL, 2); /* Discard CRC */

	return 1;						/* Return with success */
}


/**
 * 
 * @return 
 */
static uint8_t wait_ready (void)
{
	uint8_t res;


	Timer2 = 500;	/* Wait for ready in timeout of 500ms */
    spi_xchg(SDCARD_SPI_DRV, NULL, NULL, 1);
	do
		spi_xchg(SDCARD_SPI_DRV, NULL, &res, 1);
	while ((res != 0xFF) && Timer2);

	return res;
}


static void deselect (void)
{ 
    spi_cs_dis(SDCARD_SPI_DRV);
    spi_xchg(SDCARD_SPI_DRV, NULL, NULL, 1);
}


static int select (void)	/* 1:Successful, 0:Timeout */
{
	spi_cs_en(SDCARD_SPI_DRV);
	if (wait_ready() != 0xFF) {
		deselect();
		return 0;
	}
	return 1;
}

#endif
