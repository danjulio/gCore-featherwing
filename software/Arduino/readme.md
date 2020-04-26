## gCore ESP32 Arduino

Two simple demos showing how to integrate gCore into ESP32 Arduino projects.  You will need to add ESP32 support to Arduino (see links below).

1. `gcore_power_demo` - Simple sketch that includes `gcore_power.ino` for power management (see below).  The sketch simply prints battery voltage, button press detection (short/long press) and charge state every second.  After 60 seconds a long press will power off (when the button is released).  After 120 seconds the sketch powers down automatically.

2. `lvgl_demo` - A port of the LittlevGL demo to arduino.  This sketch includes `tft.ino` and `ts.ino` along with the driver integration code for the Arduino version of LittlevGL to use the Adafruit TFT/touchscreen.  The demo also uses `gcore_power.ino` to provide automatic low-battery shutdown and shutdown on a long power button press.  You will need to add the LittlevGL library to Arduino.  I used v6.1.1 during development.

### Useful Links

* [Esspressif ESP32 Arduino github](https://github.com/espressif/arduino-esp32)
* [LittlevGL Arduino github](https://github.com/littlevgl/lv_arduino)

### gcore_power module API
The ```gcore_power``` module creates a separate FreeRTOS task to read the three analog inputs connected to gCore circuitry (battery voltage, button, and charge state), keep power-management related state and provide an API for use by user Arduino code (protected by a mutex).

`gcore_set_btn_pin(pin)` - Used to set a non-standard GPIO pin for the button analog input (if you cut J3 and wire to another analog input).  Call this before calling ```gcore_begin()```.  Does not need to be called to use the default input.

`gcore_set_stat_pin(pin)` - Used to set a non-standard GPIO pin for the status analog input (if you cut J4 and wire to another analog input).  Call this before calling `gcore_begin()`.  Does not need to be called to use the default input.
 
`gcore_begin()` - Initialize the power monitoring system.  Should be called during setup before calling other API routines (aside from the set_pin routines above).

`gcore_get_batt_voltage()` - Returns a float with the current battery voltage.  Note that the ESP32 internal vref is not highly precise and although the Arduino library attempts to use any built-in calibration constants this reading can be off by 100mV or so.

`gcore_set_low_voltage_threshold(volts)` - Set the automatic low-battery shutdown voltage specifying a float between 2.5 an 4.2.  The default is 3.4 volts.

`gcore_get_low_voltage_threshold()` - Returns a float with the current automatic low-battery shutdown voltage.

`gcore_set_low_voltage_duration(secs)` - Set the automatic low-battery shutdown interval specifying an integer number of seconds the battery voltage must be below the low voltage threshold.  The default is 10 seconds.

`gcore_get_low_voltage_duration()` - Returns an integer containing the automatic low-battery shutdown interval.

`gcore_button_down()` - Returns a bool indicating if the power button is currently detected in the pressed state.

`gcore_button_short_press()` - Returns a bool indicating if a short-press has been detected.  A short-press is detected when the power button is pressed and released before the button threshold seconds have elapsed.

`gcore_button_long_press()` - Returns a bool indicating if a long-press has been detected.  A long-press is detected when the power button is pressed at the point button threshold seconds have elapsed.

`gcore_set_button_shutdown_enable(en)` - Enable or disable power-off when a long-press is detected by passing in a true or false bool value.  When enabled power will be disabled when the power button is held for the button threshold seconds.  Note that because the button circuitry enables power while the button is pressed power will not actually be removed until the button is released.  This function enables the code to de-assert PWR_HOLD.  Power button power-off functionality is enabled by default.

`gcore_get_button_shutdown_enable()` - Returns a bool indicating if power button power-off functionality is enabled or not.

`gcore_set_button_threshold_duration(secs)` - Set the button threshold integer seconds used to differentiate between short and long press.  The default value is 2 seconds.

`gcore_get_button_threshold_duration()` - Return and integer containing the button threshold seconds.

`gcore_get_charge_state()` - Returns an integer indicating the current charge state.

* 0 - Charge Idle.  No external power source connected.
* 1 - Charge Complete.  Not charging but external power source connected.
* 2 - Charging.
* 3 - Charge Fault (see the MCP73871 specification).

`gcore_power_down()` - De-assert the PWR_HOLD signal to immediately powerdown gCore.