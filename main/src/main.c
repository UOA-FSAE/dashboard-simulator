
/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#define _DEFAULT_SOURCE /* needed for usleep() */
#include <stdlib.h>
#include <unistd.h>
#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" issue*/
#include <SDL2/SDL.h>
#include "lvgl/lvgl.h"
#include "lvgl/examples/lv_examples.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_drivers/sdl/sdl.h"

// For the vehicle
#include "screens.h"
#include "vehicle.h"
#include "handle_input.h"
#include "popups.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void hal_init(void);
static int tick_thread(void *data);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_style_t speedometer_style;
static lv_style_t colored_label_style;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      VARIABLES
 **********************/

Vehicle_Data the_vehicle;

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int main(int argc, char **argv)
{
  (void)argc; /*Unused*/
  (void)argv; /*Unused*/

  char line[256];

  /*Initialize LVGL*/
  lv_init();

  /*Initialize the HAL (display, input devices, tick) for LVGL*/
  hal_init();

    the_vehicle.race.currentLapTime = 5630;
    the_vehicle.race.previousLapTime+=6892;
    the_vehicle.race.bestLapTime+=4711;
    the_vehicle.driver.rpm=15700;
    the_vehicle.race.deltaLapTime=-1262;
    the_vehicle.race.lapNumber=9;

    the_vehicle.driver.throttle=66;

    the_vehicle.ts.soc = 67;
    the_vehicle.glv.voltage = 27.8;
    the_vehicle.driver.steeringAngle = 88;
    the_vehicle.driver.frontBrakePressure = 22;
    the_vehicle.driver.rearBrakePressure = 23;
    the_vehicle.ts.minVoltage = 3.62f;
    the_vehicle.ts.maxVoltage = 3.67f;

    the_vehicle.drive[0].motorTemp = 51;
    the_vehicle.drive[0].inverterTemp = 39;
    the_vehicle.drive[1].motorTemp = 55;
    the_vehicle.drive[1].inverterTemp = 41;
    the_vehicle.drive[2].motorTemp = 54;
    the_vehicle.drive[2].inverterTemp = 42;
    the_vehicle.drive[3].motorTemp = 78;
    the_vehicle.drive[3].inverterTemp = 60;
    the_vehicle.drive[3].errorCode = 1;

    the_vehicle.errors.PDOC = true;

    the_vehicle.ts.soc=(the_vehicle.ts.soc+1)%100;
    the_vehicle.glv.voltage = the_vehicle.glv.voltage+0.1;
    if (the_vehicle.glv.voltage > 29.4) the_vehicle.glv.voltage = 22.0;

  init_screens();

  change_screens(DEBUG_SCREEN);

  int counter = 0;

  while(1) {
    /* Periodically call the lv_task handler.
     * It could be done in a timer interrupt or an OS task too.*/
    lv_timer_handler();
    try_update_screen();
//    try_enable_popups();
//    try_disable_popups();
    update_screen();
    usleep(5 * 1000);
    counter++;

    if (counter%200 == 100) {
        cycle_screens();
    }

//    the_vehicle.race.currentLapTime+=1000;
//    the_vehicle.race.previousLapTime+=2000;
//    the_vehicle.race.bestLapTime+=3000;
//    the_vehicle.driver.rpm=(the_vehicle.driver.rpm+100)%20000;
//    the_vehicle.race.deltaLapTime-=100;
//    the_vehicle.race.lapNumber=(the_vehicle.race.lapNumber+1)%100;
//
//    the_vehicle.driver.throttle=(the_vehicle.driver.throttle+2)%100;
//
//    the_vehicle.ts.soc=(the_vehicle.ts.soc+1)%100;
//    the_vehicle.glv.voltage = the_vehicle.glv.voltage+0.1;
//    if (the_vehicle.glv.voltage > 29.4) the_vehicle.glv.voltage = 22.0;

  }
  return 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the LVGL graphics
 * library
 */
static void hal_init(void)
{
  /* Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/
  sdl_init();
  /* Tick init.
   * You have to call 'lv_tick_inc()' in periodically to inform LittelvGL about
   * how much time were elapsed Create an SDL thread to do this*/
  SDL_CreateThread(tick_thread, "tick", NULL);

  /*Create a display buffer*/
  static lv_disp_draw_buf_t disp_buf1;
  static lv_color_t buf1_1[MONITOR_HOR_RES * 100];
  static lv_color_t buf1_2[MONITOR_HOR_RES * 100];
  lv_disp_draw_buf_init(&disp_buf1, buf1_1, buf1_2, MONITOR_HOR_RES * 100);

  /*Create a display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv); /*Basic initialization*/
  disp_drv.draw_buf = &disp_buf1;
  disp_drv.flush_cb = sdl_display_flush;
  disp_drv.hor_res = MONITOR_HOR_RES;
  disp_drv.ver_res = MONITOR_VER_RES;
  disp_drv.antialiasing = 1;

  lv_disp_t * disp = lv_disp_drv_register(&disp_drv);

  lv_theme_t * th = lv_theme_default_init(disp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), LV_THEME_DEFAULT_DARK, LV_FONT_DEFAULT);
  lv_disp_set_theme(disp, th);

  lv_group_t * g = lv_group_create();
  lv_group_set_default(g);

  /* Add the mouse as input device
   * Use the 'mouse' driver which reads the PC's mouse*/
//   mouse_init();
  static lv_indev_drv_t indev_drv_1;
  lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
  indev_drv_1.type = LV_INDEV_TYPE_POINTER;

  /*This function will be called periodically (by the library) to get the mouse position and state*/
  indev_drv_1.read_cb = sdl_mouse_read;
  lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv_1);

  // keyboard_init();
  static lv_indev_drv_t indev_drv_2;
  lv_indev_drv_init(&indev_drv_2); /*Basic initialization*/
  indev_drv_2.type = LV_INDEV_TYPE_KEYPAD;
  indev_drv_2.read_cb = sdl_keyboard_read;
  lv_indev_t *kb_indev = lv_indev_drv_register(&indev_drv_2);
  lv_indev_set_group(kb_indev, g);
  // mousewheel_init();
  static lv_indev_drv_t indev_drv_3;
  lv_indev_drv_init(&indev_drv_3); /*Basic initialization*/
  indev_drv_3.type = LV_INDEV_TYPE_ENCODER;
  indev_drv_3.read_cb = sdl_mousewheel_read;

  lv_indev_t * enc_indev = lv_indev_drv_register(&indev_drv_3);
  lv_indev_set_group(enc_indev, g);

  /*Set a cursor for the mouse*/
  LV_IMG_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
  lv_obj_t * cursor_obj = lv_img_create(lv_scr_act()); /*Create an image object for the cursor */
  lv_img_set_src(cursor_obj, &mouse_cursor_icon);           /*Set the image source*/
  lv_indev_set_cursor(mouse_indev, cursor_obj);             /*Connect the image  object to the driver*/
}

/**
 * A task to measure the elapsed time for LVGL
 * @param data unused
 * @return never return
 */
static int tick_thread(void *data) {
  (void)data;

  while(1) {
    SDL_Delay(5);
    lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
  }

  return 0;
}
