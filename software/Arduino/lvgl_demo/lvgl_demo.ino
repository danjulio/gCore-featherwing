/*
 * LittlevGL Arduino Demo ported to gCore
 * 
 * This sketch integrates gCore supporting software
 *   1. gcore_power
 *   2. HX8357 TFT LittlevGL Driver
 *   3. STMPE610 Resistive Touch LittlevGL Driver
 *   
 * It requires the LittlevGL Arduino library to be installed.  See https://github.com/littlevgl/lv_arduino
 * 
 * Compile for ESP WROVER Module
 *    Flash Mode: "DIO"
 *    Flash Frequency: "40 MHz"
 *    Partition Scheme: Default 4MB (could be others depending on application requirements)
 * 
 * Note: The Arduino ESP32 experimental library must be used instead of the released
 * package at the time this module was written in April 2020 (release version 1.0.4
 * does not include the analogReadMilliVolts function).  See instructions for installation
 * of the development repository at https://github.com/espressif/arduino-esp32
 * 
 */
#include "SPI.h"
#include <Ticker.h>
#include <lvgl.h>


// ==================================================
// Constants
//
#define SPI_SCK    5
#define SPI_MOSI   18
#define SPI_MISO   19

#define SD_CS      14

//
// gCore pins
//


//
// LittlevGL Configuration
//
#define LVGL_TICK_PERIOD 20

#define LV_DEMO_WALLPAPER 1
#define USE_LV_DEMO 1
//#define LV_DEMO_SLIDE_SHOW 1



// ==================================================
// Variables
//

Ticker tick; /* timer for interrupt handler */

// LittlevGL display working buffer
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

bool hw_ready = false;


// ==================================================
// Subroutines
//
#if USE_LV_LOG != 0
/* Serial debugging */
void hal_print(lv_log_level_t level, const char * file, uint32_t line, const char * dsc)
{
  Serial.printf("%s@%d->%s\r\n", file, line, dsc);
  delay(100);
}
#endif


void hal_init(void)
{
  #if USE_LV_LOG != 0
  lv_log_register_print(hal_print); /* register print function for debugging */
  #endif

  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);
  
  /*Initialize the display*/
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 480;
  disp_drv.ver_res = 320;
  disp_drv.flush_cb = littlevgl_disp_flush;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);
  
  /*Initialize the touch pad*/
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = littlevgl_tp_read;
  lv_indev_drv_register(&indev_drv);
}


// Interrupt driven periodic handler
static void lv_tick_handler(void)
{
  lv_tick_inc(LVGL_TICK_PERIOD);
}


bool hardware_init(void)
{
  // Initialize gcore_power for power hold, battery monitoring and button soft-off
  gcore_begin();
  
  // Force the SD CS High
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  Serial.begin(115200);
  
  // SPI setup here since it's used by both the LCD and Touchpad
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  
  // LCD
  tft_begin();

  // Touchpad
  ts_begin();

  return true;
}



// ==================================================
// Arduino Entry points
//

void setup() {
  hw_ready = hardware_init();

  if (hw_ready) {
    lv_init();

    hal_init();

    // Initialize the graphics library's tick
    tick.attach_ms(LVGL_TICK_PERIOD, lv_tick_handler);

    demo_create();
  }

}


void loop() {
  // put your main code here, to run repeatedly:

  if (hw_ready) {
    lv_task_handler();
  }

  delay(LVGL_TICK_PERIOD);
}
