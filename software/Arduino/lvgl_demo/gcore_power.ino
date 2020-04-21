/*
 * gCore Power Management Functions
 *
 * Provides the following functions:
 *
 *  1. Power button management
 *      - Configurable Short / Long press detection
 *      - Optional power-off on long-press
 *  2. Battery monitoring
 *      - Battery voltage
 *      - Configurable low-battery auto shutdown
 *  3. Charge State monitoring
 *  
 * Note: The Arduino ESP32 experimental library must be used instead of the released
 * package at the time this module was written in April 2020 (release version 1.0.4
 * does not include the analogReadMilliVolts function).  See instructions for installation
 * of the development repository at https://github.com/espressif/arduino-esp32
 *
 * Copyright (c) 2020 Dan Julio (dan@danjuliodesigns.com)
 *
 * gCore power management library is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * gCore power management library is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.
 *
 * See <http://www.gnu.org/licenses/>.
 *
 */

 
// ================================================================================
// Constants
// ================================================================================

//
// Default IO Pins
//   Power Hold connected to IO2
//   SNS_BATT connected to IO35/ADC1_CH7
//   SNS_BTN connected to IO34/ADC1_CH6
//   SNS_STAT connected to IO36/ADC1_CH0
//
#define GCORE_PWR_HOLD 2
#define GCORE_SNS_BATT 35
#define GCORE_SNS_BTN  34
#define GCORE_SNS_STAT 36

//
// Parameters to convert ADC readings to voltage
//

// Battery
//   Measured through a resistor divider with 5.02 scaling Vbatt = 5.02 * Vadc
//   Vadc max ~ 838 mV ==> Can use 0db attenuation on ADC input for maximum range
#define GCORE_BATT_ADC_MULT 5.02
#define GCORE_BATT_ATTEN    ADC_0db

// Power Button
//   Measured through a resistor divider with 5.02 scaling Vbatt = 5.02 * Vadc
//   Vadc max ~ 1000 mV ==> Can use 0db attenuation on ADC input for maximum range
#define GCORE_BTN_ADC_MULT  5.02
#define GCORE_BTN_ATTEN     ADC_0db

// Status Input
//   Directly measured
//   Vadc max ~ 3300 mV ==> Use 11db attenutation on ADC input for maximum range
#define GCORE_STAT_ATTEN    ADC_11db


//
// Default configuration settings
//
#define GCORE_LOW_BATT       3.4
#define GCORE_LOW_BATT_TO    10
#define GCORE_LONG_PRESS_TO  2
#define GCORE_LONG_PRESS_EN  true

//
// Battery threshold limits (volts)
//
#define GCORE_MIN_LOW_BATT   2.5
#define GCORE_MAX_LOW_BATT   4.2

//
// Button detection threshold (mV)
//   - Button detected pressed when input measured above this value
//
#define GCORE_BTN_THRESH_MV  1500

//
// Task evaluation period (mSec)
//
#define GCORE_EVAL_MSEC 50
#define GCORE_EVAL_PER_SEC (1000 / GCORE_EVAL_MSEC)

//
// Averaging buffer lengths
//
#define GCORE_BATT_AVG_NUM 20




// ================================================================================
// Enums
// ================================================================================

//
// Power Button processing state
//
typedef enum
{
  WAIT_FOR_RELEASE,
  NOT_PRESSED,
  PRESS_SHORT,
  PRESS_LONG
} gcore_btn_t;


//
// Charge state
//
typedef enum
{
  CHARGE_IDLE,
  CHARGE_COMPLETE,
  CHARGE_IN_PROGRESS,
  CHARGE_FAULT
} gcore_charge_t;



// ================================================================================
// Data structure for sharing between our task and the object
// ================================================================================
struct gcore_vars_type
{
  float batt_v;                      // Current battery voltage
  float low_batt_v;                  // Low battery detection threshold
  int low_volt_t;                    // Low battery detection timeout (sec)
  
  bool button_down;                  // Current button pressed status
  bool button_short_press;           // Set when short press detected, cleared when read by API
  bool button_long_press;            // Set when long press detected, cleared when read by API
  bool button_shutdown_en;           // Set to enable shutdown on long-press detection
  int button_threshold_t;            // Button down threshold between short and long press (sec)
    
  gcore_charge_t charge_state;    // Current charge state
};



// ================================================================================
// Variables
// ================================================================================

//
// Task-related variables
//
int gcore_batt_pin = GCORE_SNS_BATT;
int gcore_btn_pin = GCORE_SNS_BTN;
int gcore_stat_pin = GCORE_SNS_STAT;

bool gcore_enable_btn = false;
bool gcore_enable_stat = false;

int gcore_batt_avg_array[GCORE_BATT_AVG_NUM]; // mV readings
int gcore_batt_avg_index;

gcore_btn_t gcore_btn_state;


//
// Variables shared between the task and API
//
SemaphoreHandle_t gcore_mutex = NULL;
struct gcore_vars_type gcore_vars;


//
// API-related variables
//


// ================================================================================
// API routines
// ================================================================================

// Call before gcore_begin(), set to NULL to disable
void gcore_set_btn_pin(int pin_num)
{
  gcore_btn_pin = pin_num;
}


// Call before gcore_begin(), set to NULL to disable
void gcore_set_stat_pin(int pin_num)
{
  gcore_stat_pin = pin_num;
}


// Call immediately from begin() to set PWR_HOLD
bool gcore_begin()
{
  int i;
  int adc_mv;
  
  // Immediately assert PWR_HOLD to keep the system powered when the power button is released
  pinMode(GCORE_PWR_HOLD, OUTPUT);
  digitalWrite(GCORE_PWR_HOLD, HIGH);

  // Determine if the BUTTON and STAT inputs should be monitored
  gcore_enable_btn = (gcore_btn_pin != NULL) ? true : false;
  gcore_enable_stat = (gcore_stat_pin != NULL) ? true : false;

  // Configure the ADC for each input
  analogSetPinAttenuation(gcore_batt_pin, GCORE_BATT_ATTEN);
  if (gcore_enable_btn) {
    analogSetPinAttenuation(gcore_btn_pin, GCORE_BTN_ATTEN);
  }
  if (gcore_enable_stat) {
    analogSetPinAttenuation(gcore_stat_pin, GCORE_STAT_ATTEN);
  }

  // Get some initial readings
  adc_mv = analogReadMilliVolts(gcore_batt_pin);
  gcore_batt_avg_index = 0;
  for (i=0; i<GCORE_BATT_AVG_NUM; i++) {
    gcore_batt_avg_array[i] = adc_mv;
  }
  gcore_vars.batt_v = _gcore_avg_batt_v();
  gcore_vars.low_batt_v = GCORE_LOW_BATT;
  gcore_vars.low_volt_t = GCORE_LOW_BATT_TO;

  if (gcore_enable_btn) {
    adc_mv = analogReadMilliVolts(gcore_btn_pin);
    if (_gcore_btn_v(adc_mv) >= GCORE_BTN_THRESH_MV) {
      gcore_btn_state = WAIT_FOR_RELEASE;
      gcore_vars.button_down = true;
    } else {
      gcore_btn_state = NOT_PRESSED;
      gcore_vars.button_down = false;
    }
  } else {
    gcore_btn_state = NOT_PRESSED;
    gcore_vars.button_down = false;
  }
  gcore_vars.button_short_press = false;
  gcore_vars.button_long_press = false;
  gcore_vars.button_shutdown_en = GCORE_LONG_PRESS_EN;
  gcore_vars.button_threshold_t = GCORE_LONG_PRESS_TO;

  if (gcore_enable_stat) {
    adc_mv = analogReadMilliVolts(gcore_stat_pin);
    gcore_vars.charge_state = (gcore_charge_t) _gcore_compute_charge_state(adc_mv);
  } else {
    gcore_vars.charge_state = CHARGE_IDLE;
  }

  // Start the monitoring task
  gcore_mutex = xSemaphoreCreateBinary();
  xSemaphoreGive(gcore_mutex);
  xTaskCreate(_gcore_mon_task, "gCore Monitor", 10000, NULL, 1, NULL);
}

    
float gcore_get_batt_voltage()
{
  float f;

  xSemaphoreTake(gcore_mutex, portMAX_DELAY);
  f = gcore_vars.batt_v;
  xSemaphoreGive(gcore_mutex);

  return f;
}


void gcore_set_low_voltage_threshold(float thresh)
{
  if (thresh > GCORE_MAX_LOW_BATT) {
    thresh = GCORE_MAX_LOW_BATT;
  } else if (thresh < GCORE_MIN_LOW_BATT) {
    thresh = GCORE_MIN_LOW_BATT;
  }
  
  xSemaphoreTake(gcore_mutex, portMAX_DELAY);
  gcore_vars.low_batt_v = thresh;
  xSemaphoreGive(gcore_mutex);
}


float gcore_get_low_voltage_threshold()
{
  float f;
  
  xSemaphoreTake(gcore_mutex, portMAX_DELAY);
  f = gcore_vars.low_batt_v;
  xSemaphoreGive(gcore_mutex);

  return f;
}


void gcore_set_low_voltage_duration(int sec)
{
  xSemaphoreTake(gcore_mutex, portMAX_DELAY);
  gcore_vars.low_volt_t = sec;
  xSemaphoreGive(gcore_mutex);
}


int gcore_get_low_voltage_duration()
{
  int i;
  
  xSemaphoreTake(gcore_mutex, portMAX_DELAY);
  i = gcore_vars.low_volt_t;
  xSemaphoreGive(gcore_mutex);

  return i;
}

    
bool gcore_button_down()
{
  bool b;
  
  xSemaphoreTake(gcore_mutex, portMAX_DELAY);
  b = gcore_vars.button_down;
  gcore_vars.button_down = false;
  xSemaphoreGive(gcore_mutex);

  return b;
}


bool gcore_button_short_press()
{
  bool b;
  
  xSemaphoreTake(gcore_mutex, portMAX_DELAY);
  b = gcore_vars.button_short_press;
  gcore_vars.button_short_press = false;
  xSemaphoreGive(gcore_mutex);

  return b;
}


bool gcore_button_long_press()
{
  bool b;
  
  xSemaphoreTake(gcore_mutex, portMAX_DELAY);
  b = gcore_vars.button_long_press;
  gcore_vars.button_long_press = false;
  xSemaphoreGive(gcore_mutex);

  return b;
}


void gcore_set_button_shutdown_enable(bool en)
{
  xSemaphoreTake(gcore_mutex, portMAX_DELAY);
  gcore_vars.button_shutdown_en = en;
  xSemaphoreGive(gcore_mutex);
}


bool gcore_get_button_shutdown_enable()
{
  bool b;
  
  xSemaphoreTake(gcore_mutex, portMAX_DELAY);
  b = gcore_vars.button_shutdown_en;
  xSemaphoreGive(gcore_mutex);

  return b;
}


void gcore_set_button_threshold_duration(int sec)
{
  xSemaphoreTake(gcore_mutex, portMAX_DELAY);
  gcore_vars.button_threshold_t = sec;
  xSemaphoreGive(gcore_mutex);
}


int gcore_get_button_threshold_duration()
{
  int i;
  
  xSemaphoreTake(gcore_mutex, portMAX_DELAY);
  i = gcore_vars.button_threshold_t;
  xSemaphoreGive(gcore_mutex);

  return i;
}

    
int gcore_get_charge_state()
{
  int i;
  
  xSemaphoreTake(gcore_mutex, portMAX_DELAY);
  i = (int) gcore_vars.charge_state;
  xSemaphoreGive(gcore_mutex);

  return i;
}

    
void gcore_power_down()
{
  // Kill our power
  digitalWrite(GCORE_PWR_HOLD, LOW);
}




// ================================================================================
// Internal routines
// ================================================================================

// Monitoring task
void _gcore_mon_task(void* parameter)
{
  // Local task variables
  int adc_mv;
  
  float cur_batt_v;
  float cur_low_batt_v;
  int cur_low_volt_t;
  int low_volt_count = GCORE_EVAL_PER_SEC * GCORE_LOW_BATT_TO;

  bool cur_button;
  bool prev_button = gcore_vars.button_down;
  bool cur_button_down = gcore_vars.button_down;
  bool cur_button_shutdown_en;
  bool button_pressed;
  bool button_released;
  bool button_short_press_detected;
  bool button_long_press_detected;
  int cur_button_threshold_t;
  int button_down_count;

  gcore_charge_t cur_charge_state;
  
  while (1) {
    //
    // Get this evaluation's control values
    //
    
    xSemaphoreTake(gcore_mutex, portMAX_DELAY);
    
    cur_low_batt_v = gcore_vars.low_batt_v;
    cur_low_volt_t = gcore_vars.low_volt_t;

    if (gcore_enable_btn) {
      cur_button_shutdown_en = gcore_vars.button_shutdown_en;
      cur_button_threshold_t = gcore_vars.button_threshold_t;
    }
    xSemaphoreGive(gcore_mutex);
    
    //
    // Make measurements
    //
    adc_mv = analogReadMilliVolts(gcore_batt_pin);
    gcore_batt_avg_array[gcore_batt_avg_index] = adc_mv;
    if (++gcore_batt_avg_index >= GCORE_BATT_AVG_NUM) gcore_batt_avg_index = 0;
    cur_batt_v = _gcore_avg_batt_v();
    
    if (gcore_enable_btn) {
      adc_mv = analogReadMilliVolts(gcore_btn_pin);
      cur_button = (_gcore_btn_v(adc_mv) >= GCORE_BTN_THRESH_MV);
    }

    if (gcore_enable_stat) {
      adc_mv = analogReadMilliVolts(gcore_stat_pin);
      cur_charge_state = (gcore_charge_t) _gcore_compute_charge_state(adc_mv);
    }
    
    //
    // Evaluate battery condition
    //
    if (cur_batt_v < cur_low_batt_v) {
      if (--low_volt_count == 0) {
        // Power down
        digitalWrite(GCORE_PWR_HOLD, LOW);
      }
    } else {
      // Hold timer in reset
      low_volt_count = GCORE_EVAL_PER_SEC * cur_low_volt_t;
    }
    
    //
    // Evaluate switch state
    //
    if (gcore_enable_btn) {
      // Debounce and detect changes
      button_pressed = false;   // Will be set as necessary
      button_released = false;
      button_short_press_detected = false;
      button_long_press_detected = false;
      if (!cur_button_down && cur_button && prev_button) {
        button_pressed = true;
        cur_button_down = true;
      }
      if (cur_button_down && !cur_button && !prev_button) {
        button_released = true;
        cur_button_down = false;
      }
      prev_button = cur_button;

      // Update button state
      switch (gcore_btn_state) {
        case WAIT_FOR_RELEASE:
          if (button_released) {
            gcore_btn_state = NOT_PRESSED;
          }
          break;
        case NOT_PRESSED:
          if (button_pressed) {
            gcore_btn_state = PRESS_SHORT;
            button_down_count = GCORE_EVAL_PER_SEC * cur_button_threshold_t;
          }
          break;
        case PRESS_SHORT:
          if (button_released) {
            // Short press detected
            gcore_btn_state = NOT_PRESSED;
            button_short_press_detected = true;
          } else {
            if (--button_down_count == 0) {
              // Long press detected
              if (cur_button_shutdown_en) {
                // Shut down on long press
                digitalWrite(GCORE_PWR_HOLD, LOW);
              } else {
                // Note long press
                gcore_btn_state = PRESS_LONG;
                button_long_press_detected = true;
              }
            }
          }
          break;
        case PRESS_LONG:
          // Wait for release
          if (button_released) {
            gcore_btn_state = NOT_PRESSED;
          }
          break;
      }
    }

    //
    // Update this evaulation's state
    //
    xSemaphoreTake(gcore_mutex, portMAX_DELAY);
    gcore_vars.batt_v = cur_batt_v;

    if (gcore_enable_btn) {
      gcore_vars.button_down = cur_button_down;
      if (button_short_press_detected) {
        gcore_vars.button_short_press = true;
      }
      if (button_long_press_detected) {
        gcore_vars.button_long_press = true;
      }
    }

    if (gcore_enable_stat) {
      gcore_vars.charge_state = cur_charge_state;
    }
    xSemaphoreGive(gcore_mutex);

    // Sleep
    vTaskDelay(GCORE_EVAL_MSEC / portTICK_RATE_MS);
  }
}

// Convert the mv readings in the averaging buffer into a battery voltage
//   Multiply to account for hardware resistor divider
float _gcore_avg_batt_v()
{
  int i;
  int sum = 0;

  for (i=0; i<GCORE_BATT_AVG_NUM; i++) {
    sum += gcore_batt_avg_array[i];
  }

  return (GCORE_BATT_ADC_MULT * (((float) sum) / GCORE_BATT_AVG_NUM) / 1000.0);
}


// Convert a mv reading to hardware mv for the power button
int _gcore_btn_v(int adc_mv)
{
  return round(GCORE_BTN_ADC_MULT * adc_mv);
}


//  STAT2   STAT1    NomV    State
//  ------------------------------------------------
//    H       H      3.3v    Charge Idle
//    H       L      1.67v   Charging
//    L       H      1.98v   Charge Complete
//    L       L      1.24v   Charge Fault
int _gcore_compute_charge_state(int adc_mv)
{
  if (adc_mv > 2500) {
    return (int) CHARGE_IDLE;
  } else if (adc_mv > 1850) {
    return (int) CHARGE_COMPLETE;
  } else if (adc_mv > 1450) {
    return (int) CHARGE_IN_PROGRESS;
  }
  return (int) CHARGE_FAULT;
}
