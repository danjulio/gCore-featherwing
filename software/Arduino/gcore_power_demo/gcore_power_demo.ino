/*
 * Demo/test program for gcore_power module for Arduino
 * 
 * Reports battery voltage, switch and charge state every second via the USB serial port.  After 60 seconds enables
 * long-press to disable power.  Powers down at 120 seconds.
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
void setup() {
  Serial.begin(115200);
  Serial.printf("gcore_power_demo\n");
  gcore_begin();
  gcore_set_button_shutdown_enable(false);

  Serial.printf("Battery cut-off = %1.2fv\n", gcore_get_low_voltage_threshold());
  Serial.printf("Battery cut-off duration = %d sec\n", gcore_get_low_voltage_duration());
  Serial.printf("Button short-long press threshold = %d sec\n", gcore_get_button_threshold_duration());
  Serial.printf("\n");
}

void loop() {
  static int sec_count = 0;

  while (1) {
    Serial.printf("%d:\n", sec_count);
    Serial.printf("  Battery = %1.2fv\n", gcore_get_batt_voltage());

    Serial.printf("  Button Down = %d\n", gcore_button_down());
    if (gcore_button_short_press()) {
      Serial.printf("    Short Press Detected\n");
    } else if (gcore_button_long_press()) {
      Serial.printf("    Long Press Detected\n");
    }

    Serial.printf("  Charge State = ");
    switch (gcore_get_charge_state()) {
      case 0:
        Serial.printf("IDLE\n");
        break;
      case 1:
        Serial.printf("COMPLETE\n");
        break;
      case 2:
        Serial.printf("IN PROGRESS\n");
        break;
      case 3:
        Serial.printf("FAULT\n");
        break;
      default:
        Serial.printf("UNKNOWN\n");
    }
    
    delay(1000);
    ++sec_count;
    if (sec_count == 60) {
      gcore_set_button_shutdown_enable(true);
    } else if (sec_count == 120) {
      gcore_power_down();
    }
  }
}
