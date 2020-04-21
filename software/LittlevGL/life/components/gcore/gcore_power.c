/**
 *
 * gcore_power.c - gCore Power Management routines
 *
 */
#include <math.h>
#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_adc_cal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "gcore_power.h"

// ================================================================================
// Local constants
// ================================================================================

#define TAG        "gCore"


//
// Parameters to convert ADC readings to voltage
//

// Battery
//   Measured through a resistor divider with 5.02 scaling Vbatt = 5.02 * Vadc
//   Vadc max ~ 838 mV ==> Can use 0db attenuation on ADC input for maximum range
#define GCORE_BATT_ADC_MULT 5.02
#define GCORE_BATT_ATTEN    ADC_ATTEN_DB_0

// Power Button
//   Measured through a resistor divider with 5.02 scaling Vbatt = 5.02 * Vadc
//   Vadc max ~ 1000 mV ==> Can use 0db attenuation on ADC input for maximum range
#define GCORE_BTN_ADC_MULT  5.02
#define GCORE_BTN_ATTEN     ADC_ATTEN_DB_0

// Status Input
//   Directly measured
//   Vadc max ~ 3300 mV ==> Use 11db attenutation on ADC input for maximum range
#define GCORE_STAT_ATTEN    ADC_ATTEN_DB_11



// ================================================================================
// Private Enums
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




// ================================================================================
// Variables
// ================================================================================


//
// Data structure for sharing between our task and the object
//
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

//
// Task-related variables
//
static TaskHandle_t gcore_task_handle = NULL;

static int gcore_batt_pin = GCORE_SNS_BATT;
static int gcore_btn_pin = GCORE_SNS_BTN;
static int gcore_stat_pin = GCORE_SNS_STAT;

static int gcore_batt_adc_ch;
static int gcore_btn_adc_ch;
static int gcore_stat_adc_ch;

static bool gcore_enable_btn = false;
static bool gcore_enable_stat = false;

static int gcore_batt_avg_array[GCORE_BATT_AVG_NUM]; // mV readings
static int gcore_batt_avg_index;

static gcore_btn_t gcore_btn_state;

//
// ADC Characterization
//
static esp_adc_cal_characteristics_t *gcore_adc_chars = NULL;
static esp_adc_cal_characteristics_t *gcore_adc_chars2 = NULL;

//
// GPIO to ADC1 Channel mapping
//
static const adc1_channel_t gcore_gpio_2_adc_ch[40] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1,  4,  5,  6,  7,  0,  1,  2,  3
};

//
// Variables shared between the task and API
//
static SemaphoreHandle_t gcore_mutex = NULL;
static struct gcore_vars_type gcore_vars;



// ================================================================================
// Forward Declarations for internal routines
// ================================================================================
void _gcore_mon_task(void* parameter);
float _gcore_avg_batt_v();
int _gcore_btn_v(int adc_mv);
gcore_charge_t _gcore_compute_charge_state(int adc_mv);



// ================================================================================
// API Routines
// ================================================================================

// Call before gcore_begin(), set to NULL to disable
void gcore_set_btn_pin(int pin_num)
{
	if ((pin_num < 0) || (pin_num > 39)) {
		gcore_btn_pin = -1;
	} else {
		if (gcore_gpio_2_adc_ch[pin_num] == -1) {
			gcore_btn_pin = -1;
		} else {
			gcore_btn_pin = pin_num;
		}
	}
}


// Call before gcore_begin(), set to NULL to disable
void gcore_set_stat_pin(int pin_num)
{
	if ((pin_num < 0) || (pin_num > 39)) {
		gcore_stat_pin = -1;
	} else {
		if (gcore_gpio_2_adc_ch[pin_num] == -1) {
			gcore_stat_pin = -1;
		} else {
			gcore_stat_pin = pin_num;
		}
	}
}


// Call immediately from begin() to set PWR_HOLD
void gcore_begin()
{
	int i;
	int adc_mv;
  
	// Immediately assert PWR_HOLD to keep the system powered when the power button is released
	gpio_set_direction(GCORE_PWR_HOLD, GPIO_MODE_OUTPUT);
	gpio_set_level(GCORE_PWR_HOLD, 1);
	
	ESP_LOGI(TAG, "Initialization");
	
	// Set the analog channels associated with the specified gpio pins and 
	// determine if the BUTTON and STAT inputs should be monitored.
	gcore_batt_adc_ch = gcore_gpio_2_adc_ch[gcore_batt_pin];
	if (gcore_btn_pin != -1) {
		gcore_btn_adc_ch = gcore_gpio_2_adc_ch[gcore_btn_pin];
		gcore_enable_btn = true;
	} else {
		gcore_enable_btn = false;
	}
	if (gcore_stat_pin != -1) {
		gcore_stat_adc_ch = gcore_gpio_2_adc_ch[gcore_stat_pin];
		gcore_enable_stat = true;
	} else {
		gcore_enable_stat = false;
	}

	// Configure the ADC for each input
	adc1_config_width(ADC_WIDTH_BIT_12);
	adc1_config_channel_atten(gcore_batt_adc_ch, GCORE_BATT_ATTEN);
	if (gcore_enable_btn) {
		adc1_config_channel_atten(gcore_btn_adc_ch, GCORE_BTN_ATTEN);
	}
	if (gcore_enable_stat) {
		adc1_config_channel_atten(gcore_stat_adc_ch, GCORE_STAT_ATTEN);
	}
	
	// Characterize ADC1 for highest accuracy
	if (gcore_adc_chars == NULL) {
		gcore_adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	}
	if (gcore_adc_chars2 == NULL) {
		gcore_adc_chars2 = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	}
	esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_0, ADC_WIDTH_BIT_12, 1100, gcore_adc_chars);
	if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        ESP_LOGI(TAG, "ADC Cal: eFuse Vref");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        ESP_LOGI(TAG, "ADC Cal: Two Point");
    } else {
        ESP_LOGI(TAG, "ADC Cal: Default");
    }
    val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, gcore_adc_chars2);

	// Get some initial readings
	adc_mv = esp_adc_cal_raw_to_voltage(adc1_get_raw(gcore_batt_adc_ch), gcore_adc_chars);
	gcore_batt_avg_index = 0;
	for (i=0; i<GCORE_BATT_AVG_NUM; i++) {
		gcore_batt_avg_array[i] = adc_mv;
	}
	gcore_vars.batt_v = _gcore_avg_batt_v();
	gcore_vars.low_batt_v = GCORE_LOW_BATT;
	gcore_vars.low_volt_t = GCORE_LOW_BATT_TO;

	if (gcore_enable_btn) {
		adc_mv = esp_adc_cal_raw_to_voltage(adc1_get_raw(gcore_btn_adc_ch), gcore_adc_chars);
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
		adc_mv = esp_adc_cal_raw_to_voltage(adc1_get_raw(gcore_stat_adc_ch), gcore_adc_chars2);
		gcore_vars.charge_state = _gcore_compute_charge_state(adc_mv);
	} else {
		gcore_vars.charge_state = CHARGE_IDLE;
	}

	// Start the monitoring task
	if (gcore_mutex == NULL) {
		gcore_mutex = xSemaphoreCreateBinary();
	}
	xSemaphoreGive(gcore_mutex);
	if (gcore_task_handle == NULL) {
		xTaskCreate(_gcore_mon_task, "gCore Monitor", 10000, NULL, 1, &gcore_task_handle);
	}
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

    
gcore_charge_t gcore_get_charge_state()
{
	gcore_charge_t s;
  
	xSemaphoreTake(gcore_mutex, portMAX_DELAY);
	s = gcore_vars.charge_state;
	xSemaphoreGive(gcore_mutex);

	return s;
}

    
void gcore_power_down()
{
	// Kill our power
	gpio_set_level(GCORE_PWR_HOLD, 0);
}




// ================================================================================
// Internal Routines
// ================================================================================

// Monitoring task
void _gcore_mon_task(void* parameter)
{
	// Local task variables
	int adc_mv;
  
	float cur_batt_v = 0;
	float cur_low_batt_v = 0;
	int cur_low_volt_t = 0;
	int low_volt_count = GCORE_EVAL_PER_SEC * GCORE_LOW_BATT_TO;

	bool cur_button = false;
	bool prev_button = gcore_vars.button_down;
	bool cur_button_down = gcore_vars.button_down;
	bool cur_button_shutdown_en = false;
	bool button_pressed = false;
	bool button_released = false;
	bool button_short_press_detected = false;
	bool button_long_press_detected = false;
	int cur_button_threshold_t = 0;
	int button_down_count = 0;

	gcore_charge_t cur_charge_state = CHARGE_IDLE;
  
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
		adc_mv = esp_adc_cal_raw_to_voltage(adc1_get_raw(gcore_batt_adc_ch), gcore_adc_chars);
		gcore_batt_avg_array[gcore_batt_avg_index] = adc_mv;
		if (++gcore_batt_avg_index >= GCORE_BATT_AVG_NUM) gcore_batt_avg_index = 0;
		cur_batt_v = _gcore_avg_batt_v();
    
		if (gcore_enable_btn) {
			adc_mv = esp_adc_cal_raw_to_voltage(adc1_get_raw(gcore_btn_adc_ch), gcore_adc_chars);
			cur_button = (_gcore_btn_v(adc_mv) >= GCORE_BTN_THRESH_MV);
		}

		if (gcore_enable_stat) {
			adc_mv = esp_adc_cal_raw_to_voltage(adc1_get_raw(gcore_stat_adc_ch), gcore_adc_chars2);
			cur_charge_state = _gcore_compute_charge_state(adc_mv);
		}
    
		//
		// Evaluate battery condition
		//
		if (cur_batt_v < cur_low_batt_v) {
			if (--low_volt_count == 0) {
				// Power down
				gpio_set_level(GCORE_PWR_HOLD, 0);
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
								gpio_set_level(GCORE_PWR_HOLD, 0);
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
gcore_charge_t _gcore_compute_charge_state(int adc_mv)
{
	if (adc_mv > 2500) {
		return CHARGE_IDLE;
	} else if (adc_mv > 1850) {
		return CHARGE_COMPLETE;
	} else if (adc_mv > 1450) {
		return CHARGE_IN_PROGRESS;
	}
	return CHARGE_FAULT;
}

