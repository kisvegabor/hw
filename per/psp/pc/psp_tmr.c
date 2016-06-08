/**
 * @file psp_tmr.c
 */

/***********************
 *       INCLUDES
 ***********************/
#include "hw_conf.h"

#if USE_TMR != 0 && PSP_PC != 0
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "hw/per/tmr.h"

/***********************
 *       DEFINES
 ***********************/
#define TMR_DEF_PERIOD 1000	/*us*/

/***********************
 *       TYPEDEFS
 ***********************/
typedef struct
{
	uint32_t period;
	void(*fp)(void);
	bool run;
}mdsc_t;

/***********************
 *   STATIC VARIABLES
 ***********************/
static mdsc_t mdsc[HW_TMR_NUM] = {
		{TMR_DEF_PERIOD, NULL, false},
		{TMR_DEF_PERIOD, NULL, false},
		{TMR_DEF_PERIOD, NULL, false},
		{TMR_DEF_PERIOD, NULL, false},
		{TMR_DEF_PERIOD, NULL, false},
		{TMR_DEF_PERIOD, NULL, false},
};

/***********************
 *   GLOBAL PROTOTYPES
 ***********************/
        
/***********************
 *   STATIC PROTOTYPES
 ***********************/
static int tmr_han(void * param);

/***********************
 *   GLOBAL FUNCTIONS
 ***********************/

void psp_tmr_init(void)
{
	uint8_t i;
	for(i = 0; i < HW_TMR_NUM; i++) {
		SDL_CreateThread(tmr_han, "tmr_han", &mdsc[i]);
	}
}

hw_res_t psp_tmr_set_period(tmr_t tmr, uint32_t p_us)
{
	mdsc[tmr].period = p_us;

	return HW_RES_OK;
}

void psp_tmr_set_cb(tmr_t tmr, void (*cb) (void))
{
	mdsc[tmr].fp = cb;
}

void psp_tmr_en_int(tmr_t tmr, bool en)
{

}

void psp_tmr_run(tmr_t tmr, bool en)
{
	mdsc[tmr].run = en;
}


/***********************
 *   STATIC FUNCTIONS
 ***********************/
static int tmr_han(void * param)
{
	mdsc_t *  my_modul = param;

	while(1) {
		if(my_modul->run != false) {
			if(my_modul->fp != NULL) {
				my_modul->fp();
				usleep(my_modul->period);
			} else {
				usleep(TMR_DEF_PERIOD);
			}
		} else {
			usleep(TMR_DEF_PERIOD);
		}
	}

	return 0;
}

#endif
