/*
 * A simple programmers calculator with number entry and conversion between decimal 
 * and hexadecimal bases with logic functions and memory.  Inspired by the Apple OS X 
 * calculator that I often use during design and programming (but with the functionality 
 * I want...).
 * 
 * A drop-down menu allows selecting the number of bits to work with (8-64 bits
 * in 8-bit increments).  A switch selects decimal or hexadecimal operation.eSPI ported to gCore
 * 
 * This code configures the display to operate in Landscape mode.  The lv_conf.h file
 * in the LVGL library directory must be configured to match
 *   #define LV_HOR_RES_MAX          (480)
 *   #define LV_VER_RES_MAX          (320)
 *
 */
#include "SPI.h"
#include <Ticker.h>
#include <lvgl.h>



// ================================================
// USER CONFIGURATION
// ================================================
// Number display color
#define READOUT_COLOR     LV_COLOR_MAKE(0xF0, 0xB0, 0x00)
#define READOUT_COLOR_LB  LV_COLOR_MAKE(0xF0, 0x00, 0x00)

// Auto-shutdown timeout (mSec)
#define APP_INACTIVITY_TIMEOUT 120000

// Low battery warning voltage - must be higher than GCORE_LOW_BATT (which turns us off)
#define APP_LOW_BATT 3.6



// ================================================
// PROGRAM CONSTANTS AND VARIABLES
// ================================================
#define APP_GCORE_EVAL_MSEC  100
#define APP_LVGL_EVAL_MSEC   20
#define APP_INACTIVITY_COUNT (APP_INACTIVITY_TIMEOUT/APP_GCORE_EVAL_MSEC)

//
// gCore pins
//
#define SPI_SCK    5
#define SPI_MOSI   18
#define SPI_MISO   19

#define SD_CS      14



// ------------------------------------------------
// Global Variables

// LVGL
lv_disp_buf_t disp_buf;
lv_color_t buf[LV_HOR_RES_MAX * 10];

const int screenWidth = 480;
const int screenHeight = 320;

// Application evaluation timers
unsigned long gcore_prev_msec;
unsigned long lvgl_prev_msec;
unsigned long app_inactivity_count;

// Low battery flag - set the first time a low-battery condition is detected
bool low_batt_flag = false;



// ================================================
// Application subroutines
// ================================================
void note_activity()
{
  app_inactivity_count = 0;
}


bool task_timeout(unsigned long* prevT, unsigned long timeout)
{
  unsigned long curT = millis();
  unsigned long deltaT;
  
  if (curT > *prevT) {
    deltaT = curT - *prevT;
  } else {
    // Handle wrap
    deltaT = ~(*prevT - curT) + 1;
  }

  if (deltaT >= timeout) {
    *prevT = curT;
    return true;
  } else {
    return false;
  }
}



// ================================================
// Arduino entry-points
// ================================================
void setup()
{
  // Diagnostic output
  Serial.begin(115200);

  // Initialize gcore_power for power hold, battery monitoring and button soft-off
  gcore_begin();
  
  // Force the SD CS High
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  // SPI setup here since it's used by both the LCD and Touchpad
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  // Setup LVGL
  lvgl_setup();

  // Setup the initial calculator values
  calc_init();

  // Draw the display (after calculator setup)
  gui_init();

  // Finally start the app
  gcore_prev_msec = millis();
  lvgl_prev_msec = gcore_prev_msec;
  app_inactivity_count = 0;
}


void loop()
{
  uint8_t reg;
  uint16_t batt_mv;
  
  // Periodically give time to LVGL to update
  if (task_timeout(&lvgl_prev_msec, 20)) {
    lv_task_handler();
  }
  
  // Periodically check gCore
  // 1. Look for conditions to power-off
  //   a. Manual power off from button press
  //   b. Inactivity shutdown (when running on battery)
  // 2. Look for low battery condition
  //
  if (task_timeout(&gcore_prev_msec, 100)) {
    if (gcore_button_short_press() ||
        ((++app_inactivity_count >= APP_INACTIVITY_COUNT) && (gcore_get_charge_state() == 0))) {
      
      Serial.println("Power down...");
      delay(10);
      gcore_power_down();
      while (1) {};
    }
  }

  if (!low_batt_flag) {
    if (gcore_get_batt_voltage() < APP_LOW_BATT) {
      low_batt_flag = true;
    }
  }
}
