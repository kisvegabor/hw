/**
 * @file tft.c
 * 
 */

/*********************
 *      INCLUDES
 *********************/
#include "hw_conf.h"
#if PSP_PC != 0 && USE_TFT != 0

#include "../psp_tft.h"
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <SDL2/SDL.h>

/*********************
 *      DEFINES
 *********************/
#define SDL_REFR_PERIOD	50	/*ms*/
#define SDL_WINDOW_FLAGS	(SDL_WINDOW_BORDERLESS)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static int sdl_refr(void * param);

/***********************
 *   GLOBAL PROTOTYPES
 ***********************/
void mouse_handler(void);

/**********************
 *  STATIC VARIABLES
 **********************/
static SDL_Window * window;
static SDL_Renderer * renderer;
static SDL_Texture * texture;
static uint32_t tft_fb[TFT_HOR_RES * TFT_VER_RES];
static int32_t last_x1;
static int32_t last_y1;
static int32_t last_x2;
static int32_t last_y2;
static bool sdl_inited = false;
static bool sdl_refr_qry = false;
static bool sdl_quit_qry = false;

int quit_filter (void *userdata, SDL_Event * event);

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the TFT
 */
hw_res_t psp_tft_init(void)
{
	hw_res_t res  = HW_RES_OK;

	SDL_CreateThread(sdl_refr, "sdl_refr", NULL);

	while(sdl_inited == false);


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
}

/**
 * Fill out the marked area with a color
 * @param color fill color
 */
void psp_tft_fill(color_t color)
{
    /*Return if the area is out the screen*/
    if(last_x2 < 0) return;
    if(last_y2 < 0) return;
    if(last_x1 > TFT_HOR_RES - 1) return;
    if(last_y1 > TFT_VER_RES - 1) return;

    /*Truncate the area to the screen*/
    int32_t act_x1 = last_x1 < 0 ? 0 : last_x1;
    int32_t act_y1 = last_y1 < 0 ? 0 : last_y1;
    int32_t act_x2 = last_x2 > TFT_HOR_RES - 1 ? TFT_HOR_RES - 1 : last_x2;
    int32_t act_y2 = last_y2 > TFT_VER_RES - 1 ? TFT_VER_RES - 1 : last_y2;

	uint32_t x;
	uint32_t y;
	uint32_t color24 = color_to24(color);

	for(x = act_x1; x <= act_x2; x++) {
		for(y = act_y1; y <= act_y2; y++) {
			tft_fb[y * TFT_HOR_RES + x] = color24 | 0xFF000000;
		}
	}

	sdl_refr_qry = true;
}

/**
 * Put a color map to the marked area
 * @param color_p an array of colors
 */
void psp_tft_map(color_t * color_p)
{
   /*Return if the area is out the screen*/
	if(last_x2 < 0) return;
	if(last_y2 < 0) return;
	if(last_x1 > TFT_HOR_RES - 1) return;
	if(last_y1 > TFT_VER_RES - 1) return;

	/*Truncate the area to the screen*/
	int32_t act_x1 = last_x1 < 0 ? 0 : last_x1;
	int32_t act_y1 = last_y1 < 0 ? 0 : last_y1;
	int32_t act_x2 = last_x2 > TFT_HOR_RES - 1 ? TFT_HOR_RES - 1 : last_x2;
	int32_t act_y2 = last_y2 > TFT_VER_RES - 1 ? TFT_VER_RES - 1 : last_y2;

	uint32_t x;
	uint32_t y;

	for(y = act_y1; y <= act_y2; y++) {
		for(x = act_x1; x <= act_x2; x++) {
			tft_fb[y * TFT_HOR_RES + x] = color_to24(*color_p) | 0xFF000000;
			color_p++;
		}

		color_p += last_x2 - act_x2;
	}

	sdl_refr_qry = true;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * SDL main thread. All SDL related task have to be handled here!
 * It initializes SDL, handles drawing and the mouse.
 */
static int sdl_refr(void * param)
{
	/*Initialize the SDL*/
    SDL_Init(SDL_INIT_VIDEO);

    SDL_SetEventFilter(quit_filter, NULL);

	window = SDL_CreateWindow("SDL2 Pixel Drawing",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		TFT_HOR_RES, TFT_VER_RES, SDL_WINDOW_FLAGS);

	renderer = SDL_CreateRenderer(window, -1, 0);
	texture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, TFT_HOR_RES, TFT_VER_RES);

	/*Initialize the frame buffer to gray (77 is an empirical value) */
	memset(tft_fb, 77, TFT_HOR_RES * TFT_VER_RES * sizeof(uint32_t));

	SDL_UpdateTexture(texture, NULL, tft_fb, TFT_HOR_RES * sizeof(uint32_t));

	sdl_inited = true;

	/*Run until quit event not arrives*/
	while(sdl_quit_qry == false) {

		/*Refresh handling*/
		if(sdl_refr_qry != false) {
			SDL_UpdateTexture(texture, NULL, tft_fb, TFT_HOR_RES * sizeof(uint32_t));
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}

#if USE_MOUSE != 0
		/*Mouse handling*/
		mouse_handler();
#endif
		/*Sleep some time*/
		usleep(SDL_REFR_PERIOD * 1000);
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	exit(0);

	return 0;


}

int quit_filter (void *userdata, SDL_Event * event)
{
	if(event->type == SDL_QUIT) {
		sdl_quit_qry = true;
	}

	return 1;
}

#endif
