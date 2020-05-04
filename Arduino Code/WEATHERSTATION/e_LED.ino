//**FUNCTIONS FOR LED MANAGEMENT**

/**
 * Function in RAM which is called when the WiFi pending timer triggers.
 * Makes the WiFi pending LED (blue) blink.
 */
void IRAM_ATTR on_wifi_pending_led_timer() {
  turn_wifi_pending_led_on();
}

/**
 * Function in RAM which is called when the setup mode timer triggers.
 * Makes the setup mode LED (green) blink.
 */
void IRAM_ATTR on_setup_mode_on_led_timer() {
  turn_setup_led_on();
}

/**
 * Turns the LED assigned to setup mode on. Bool variables enable priotization 
 * of one LED state over another, but this is currently not in use.
 */
void turn_setup_led_on() {
  batteryLowLedOn = false;
  wifiPendingLedOn = false;
  errorIndicationLedOn = false;
  dataUploadIndicationLedOn = false;

  set_led_pinout(0, 1, 0, true);
  setupOnLedOn = true;
}

/**
 * Turns the LED assigned to the low battery state on. Bool variables enable priotization 
 * of one LED state over another, but this is currently not in use.
 */
void turn_battery_low_led_on() {
  setupOnLedOn = false;
  wifiPendingLedOn = false;
  errorIndicationLedOn = false;
  dataUploadIndicationLedOn = false;

  set_led_pinout(1, 0, 0, false);
  batteryLowLedOn = true;
}

/**
 * Turns the LED assigned to the pending WiFi state on. Bool variables enable priotization 
 * of one LED state over another, but this is currently not in use.
 */
void turn_wifi_pending_led_on() {
  setupOnLedOn = false;
  batteryLowLedOn = false;
  errorIndicationLedOn = false;
  dataUploadIndicationLedOn = false;

  set_led_pinout(0, 0, 1, true);
  wifiPendingLedOn = true;
}

/**
 * Turns the LED assigned to the error indication LED on. Bool variables enable priotization 
 * of one LED state over another, but this is currently not in use.
 */
void error_indication_led_on() {
  setupOnLedOn = false;
  batteryLowLedOn = false;
  wifiPendingLedOn = false;
  dataUploadIndicationLedOn = false;

  set_led_pinout(0, 1, 1, true);
  errorIndicationLedOn = true;
}

/**
 * Turns the LED assigned to the successful data upload indication LED on. Bool variables enable priotization 
 * of one LED state over another, but this is currently not in use.
 */
void data_upload_indication_led_on() {
  setupOnLedOn = false;
  batteryLowLedOn = false;
  wifiPendingLedOn = false;
  errorIndicationLedOn = false;

  set_led_pinout(0, 1, 0, true);
  dataUploadIndicationLedOn = true;
}

/**
 * Turns all LEDs off.
 */
void turn_led_off() {
  setupOnLedOn = false;
  batteryLowLedOn = false;
  wifiPendingLedOn = false;

  set_led_pinout(0, 0, 0, false);
  errorIndicationLedOn = true;
}

/**
 * Changes the voltage levels on the LED pins of the ESP to turn LEDs on and off.
 * 
 * @param  red  voltage level to be set on the red LED pin
 * @param  green voltage level to be set on the green LED pin
 * @param  blue voltage level to be set on the blue LED pin
 * @param  blink determines whether to toggle the chosen pins
 */
void set_led_pinout(int red, int green, int blue, bool blink) {
  if (red > 0) {
    if (blink) {
      digitalWrite(RED_LED_PIN, digitalRead(RED_LED_PIN) ^ 1);
    } else {
      digitalWrite(RED_LED_PIN, 1);
    }
  } else {
    digitalWrite(RED_LED_PIN, 0);
  }
  if (green > 0) {
    if (blink) {
      digitalWrite(GREEN_LED_PIN, digitalRead(GREEN_LED_PIN) ^ 1);
    } else {
      digitalWrite(GREEN_LED_PIN, 1);
    }
  } else {
    digitalWrite(GREEN_LED_PIN, 0);
  }
  if (blue > 0) {
    if (blink) {
      digitalWrite(BLUE_LED_PIN, digitalRead(BLUE_LED_PIN) ^ 1);
    } else {
      digitalWrite(BLUE_LED_PIN, 1);
    }
  } else {
    digitalWrite(BLUE_LED_PIN, 0);
  }
}
