/**
 * @file icm20602.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "icm20602.h"

#if USE_ICM20602 != 0

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "hw/per/spi.h"
#include "hw/per/tick.h"

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
static bool ready;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void icm20602_init(void)
{
	/*Read who am I*/
	char spi_acc[10];
	spi_acc[0] = 0x75 | 0x80;
	spi_acc[1] = 0xFF;

	spi_cs_en(ICM20602_SPI_DRV);
	spi_xchg(ICM20602_SPI_DRV, spi_acc, spi_acc, 2);
	spi_cs_dis(ICM20602_SPI_DRV);

	if(spi_acc[1] == 0x12) {
		ready = true;
	} else {
		ready = false;
		return;
	}

    /*Reset*/
	spi_acc[0] = 0x6B;
	spi_acc[1] = 0x80;
	spi_cs_en(ICM20602_SPI_DRV);
	spi_xchg(ICM20602_SPI_DRV, spi_acc, spi_acc, 2);
	spi_cs_dis(ICM20602_SPI_DRV);
	tick_wait_ms(100);

	/*Reset path*/
	spi_acc[0] = 0x68;
	spi_acc[1] = 0x03;
	spi_cs_en(ICM20602_SPI_DRV);
	spi_xchg(ICM20602_SPI_DRV, spi_acc, spi_acc, 2);
	spi_cs_dis(ICM20602_SPI_DRV);
	tick_wait_ms(100);

    /*Clock auto-select*/
    spi_acc[0] = 0x6B;
	spi_acc[1] = 0x01;
	spi_cs_en(ICM20602_SPI_DRV);
	spi_xchg(ICM20602_SPI_DRV, spi_acc, spi_acc, 2);
	spi_cs_dis(ICM20602_SPI_DRV);
	tick_wait_ms(15);
}

void icm20602_read(icm20602_data_t * buf)
{
	if(ready == false) {
		memset(buf, 0, sizeof(icm20602_data_t));
		return;
	}
	char spi_cmd = 0x3B | 0x80;
	uint8_t data[14];	/*store 3 acc + 1 temp + 3 gyro data (2 bytes each)*/

	spi_cs_en(ICM20602_SPI_DRV);
	spi_xchg(ICM20602_SPI_DRV, &spi_cmd, NULL, 1);
	spi_xchg(ICM20602_SPI_DRV, NULL, data, sizeof(data));
	spi_cs_dis(ICM20602_SPI_DRV);

	buf->acc_x = (data[0] << 8) + data[1];
	buf->acc_y = (data[2] << 8) + data[3];
	buf->acc_z = (data[4] << 8) + data[5];
	buf->temp = (data[6] << 8) + data[7];
	buf->gyro_x = (data[8] << 8) + data[9];
	buf->gyro_y = (data[10] << 8) + data[11];
	buf->gyro_z = (data[12] << 8) + data[13];

}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*USE_ICM20602 != 0*/
