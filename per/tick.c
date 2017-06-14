/**
 * @file sys.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if USE_TICK != 0

#include <stddef.h>
#include "tick.h"
#include "hw/per/tmr.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void sys_time_inc(void);
#if TICK_FUNC_NUM != 0
static void (*systick_cb_a[TICK_FUNC_NUM])(void);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
volatile bool started = false;
volatile uint32_t sys_time = 0;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Init the sys tick
 * @return 
 */
void tick_init(void) {
	tmr_set_period(TICK_TIMER, 1000);
	tmr_set_cb(TICK_TIMER, sys_time_inc);
	tmr_run(TICK_TIMER, true);
}

/**
 * Get the elapsed sys. tick 
 * @return Elapsed milliseconds since system start
 */
uint32_t tick_get(void) {
	uint32_t sys_time_tmp;

	tmr_en_int(TICK_TIMER, false); /*Disable interrupt while reading*/
	sys_time_tmp = sys_time;
	tmr_en_int(TICK_TIMER, true); /*Re-enable interrupts*/

	return sys_time_tmp;
}

/**
 * Get the elapsed milliseconds since a pervious time
 * @param time_prev a pervious time stamp
 * @return 
 */
uint32_t tick_elaps(uint32_t time_prev) {
	uint32_t act_time = tick_get();

	/*If there is no overflow in sys_time
	 simple subtract*/
	if (act_time >= time_prev) {
		time_prev = act_time - time_prev;
	} else {
		time_prev = UINT32_MAX - time_prev + 1;
		time_prev += act_time;
	}

	return time_prev;
}

/**
 * Wait a given number of milliseconds
 * @param delay the desired delay in milliseconds
 */
void tick_wait_ms(uint32_t delay) {
	if (started == false) {
		uint32_t i;
		for (i = 0; i < delay; i++) {
			tick_wait_us(1000);
		}
	} else {
		uint32_t act_time = tick_get();

		while (tick_elaps(act_time) < delay) {
			tick_wait_us(100);
		}
	}
}

/**
 * Wait a given number of microseconds. 
 * Has to be adjusted with TICK_US_BASE in misc_conf.h
 * @param delay the desired delay in milliseconds
 */
void tick_wait_us(uint32_t delay) {
	volatile uint32_t i, j, k = 0;

	for (i = 0; i < delay; i++) {
		for (j = 0; j < TICK_US_BASE; j++)
			k++;
	}
}

#if TICK_FUNC_NUM != 0
/**
 * Add a callback to the systick. This function will be called in every milliseconds
 * @param fp pointer to a void func(void) function
 * @return false: too much callback. Increase TICK_FUNC_NUM in misc_conf.h
 */
bool tick_add_func(void (*fp)(void)) {
	bool suc = false;

	tmr_en_int(TICK_TIMER, false); /*Disable interrupt while reading*/

	uint8_t i;
	for (i = 0; i < TICK_FUNC_NUM; i++) {
		if (systick_cb_a[i] == NULL) {
			systick_cb_a[i] = fp;
			suc = true;
			break;
		}
	}
	tmr_en_int(TICK_TIMER, true); /*Re-enable interrupts*/

	return suc;
}

/**
 * Remove a previously added function from the systick call backs
 * @param fp pointer to sys tick callback function
 */
void tick_rem_func(void (*fp)(void)) {
	tmr_en_int(TICK_TIMER, false); /*Disable interrupt while reading*/

	uint8_t i;
	for (i = 0; i < TICK_FUNC_NUM; i++) {
		if (systick_cb_a[i] == fp) {
			systick_cb_a[i] = NULL;
			break;
		}
	}
	tmr_en_int(TICK_TIMER, true); /*Re-enable interrupts*/
}
#endif
/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Increase the sys ticks and run the call backs
 */
static void sys_time_inc(void) {
	started = true;
	sys_time++;

#if TICK_FUNC_NUM != 0
	/*Run the callback functions*/
	uint8_t i;
	for (i = 0; i < TICK_FUNC_NUM; i++) {
		if (systick_cb_a[i] != NULL) {
			systick_cb_a[i]();
		}
	}
#endif
}

#else
#if TICK_BLOCK_WAIT != 0
#include "tick.h"

/**
 * Wait a given number of milliseconds (blocking)
 * @param delay the desired delay in milliseconds
 */
void tick_wait_ms (uint32_t delay)
{
	uint32_t i;
	for(i = 0; i < delay; i++) {
		tick_wait_us(1000);
	}
}

/**
 * Wait a given number of microseconds. 
 * Has to be adjusted with TICK_US_BASE in misc_conf.h
 * @param delay the desired delay in milliseconds
 */
void tick_wait_us (uint32_t delay)
{
	volatile uint32_t i,j,k = 0;

	for(i=0;i<delay;i++) {
		for(j=0;j < TICK_US_BASE; j++) k++;
	}
}
#endif

#endif

