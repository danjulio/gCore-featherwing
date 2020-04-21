/**
 *
 * gcore_power.h - Header file for gCore Power Management routines
 *
 */
#ifndef GCORE_POWER_H_
#define GCORE_POWER_H_

#include <stdbool.h>
#include <stdint.h>


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
// API
// ================================================================================
void gcore_set_btn_pin(int pin_num);
void gcore_set_stat_pin(int pin_num);
void gcore_begin();
void gcore_power_down();

float gcore_get_batt_voltage();
void gcore_set_low_voltage_threshold(float thresh);
float gcore_get_low_voltage_threshold();
void gcore_set_low_voltage_duration(int sec);
int gcore_get_low_voltage_duration();

bool gcore_button_down();
bool gcore_button_short_press();
bool gcore_button_long_press();
void gcore_set_button_shutdown_enable(bool en);
bool gcore_get_button_shutdown_enable();
void gcore_set_button_threshold_duration(int sec);
int gcore_get_button_threshold_duration();
gcore_charge_t gcore_get_charge_state();

#endif /* GCORE_POWER_H_ */