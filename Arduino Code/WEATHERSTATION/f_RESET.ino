//**FUNCTIONS RELATED TO RESET BUTTON PRESS MANAGEMENT**

/**
 * Function in RAM which gets called when the reset button is released.
 * It checks whether the press of the button was a short or long press
 */
void IRAM_ATTR on_reset_button_state_changed() {
  int resetButtonState = digitalRead(RESET_PIN);

  if (resetButtonState == HIGH) {
    unsigned long tempTime = millis();
    if ((tempTime - lastDebounceTime) > DEBOUNCE_DELAY && resetButton.pressed == true) {
      if (tempTime - resetButton.timePressed > resetButton.longPressTime) {   //Press duration surpasses the defined time threshold
        resetButton.longPressed = true;
      } else {
        rebootRequested = true;   //Reboot as soon as possible
      }
      resetButton.pressed = false;
      resetButton.timePressed = 0;
    }
    lastDebounceTime = tempTime;
  } else {
    if (resetButton.pressed == false) {
        resetButton.pressed = true;
        lastDebounceTime = millis();
        resetButton.timePressed = millis();
        Serial.println("Detected reset button press while being awake");
      }
  }
}

/**
 * Performs a system restart if a short reset button press was detected.
 * Turns on the setup mode if a long reset button press was detected.
 */
void handle_reset_button_press() {
  //Handle short button press
  if (rebootRequested) {
    Serial.println("Reboot!");
    turn_led_off();
    ESP.restart();
  }

  //Handle long button press
  if (resetButton.longPressed) {
    Serial.println("Long button press detected!");
    timerAlarmDisable(wifiPendingLedTimer);   //Turn off WiFi pending LED
    turn_on_setup();
    start_access_point();
    resetButton.longPressed = false;
  }
}

/**
 * Function assigned to core 0 which handles reset button presses
 * 
 * @param  parameters pointer to the parameters which were passed to the function
 */
void reset_task(void *parameters) {
  while(1) {
    handle_reset_button_press();
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}


void turn_off_setup() {
  setupOn = false;
}

void turn_on_setup() {
  setupOn = true;
  timerAlarmEnable(setupModeOnLedTimer);
  Serial.println("Setup mode entered");
}

bool is_setup_on() {
  return setupOn;
}
