//**SETUP FUNCTION**

void setup() {
  
  Serial.begin(115200); //Open serial port to display data and set baudrate
  while (!Serial);      //Wait for serial to be initialized

  EEPROM.begin(EEPROM_SIZE);  //Allocate complete ESP32 flash memory storage, which is 512 Byte
  btStop();           //Turn off Bluetooth

  pinMode(RESET_PIN, INPUT_PULLUP); //Set the digital pin for the reset button
  pinMode(RED_LED_PIN, OUTPUT);     //Set the digital pin for the red LED
  pinMode(GREEN_LED_PIN, OUTPUT);   //Set the digital pin for the green LED
  pinMode(BLUE_LED_PIN, OUTPUT);    //Set the digital pin for the blue LED

  lastResetButtonState = digitalRead(RESET_PIN);  //Read the initial digital state of the reset button

  //Create Task1 and pin it to core 0
  //Loop() and Setup() run on core 1 by default
  //xTaskCreatePinnedToCore(task function, name, stack size, parameter, priority, task handle, core)
  xTaskCreatePinnedToCore(reset_task, "Task1", 10000, NULL, 1, &Task1, 0);

  //Frequency value of the base signal used by the ESP32 is 80 MHz
  //prescaler value of 80 => 80 MHz / 80 = 1 MHz <=> 1 second
  //timerBegin(number of the timer(0-3), prescaler value, count up or down flag)
  //In total there are four hardware timers
  wifiPendingLedTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(wifiPendingLedTimer, &on_wifi_pending_led_timer, true);
  timerAlarmWrite(wifiPendingLedTimer, 500000, true);  //blink every 500000 Hz (half a second)

  //Second hardware timer
  setupModeOnLedTimer = timerBegin(1, 80, true);
  timerAttachInterrupt(setupModeOnLedTimer, &on_setup_mode_on_led_timer, true);
  timerAlarmWrite(setupModeOnLedTimer, 500000, true);  //blink every 500000 Hz (half a second)

  //Two functionalities tied to the reset button
  //1) Wake the ESP up from deep sleep on button press
  //2) Register short or long button press and enter setup mode on button release (rising edge)
  // => two functionalities tied to a single button press
  attachInterrupt(resetButton.PIN, on_reset_button_state_changed, CHANGE);

  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
    if (resetButton.pressed == false) {
      resetButton.pressed = true;
      lastDebounceTime = millis();
      resetButton.timePressed = millis();
      Serial.println("Detected reset button press while in deep sleep mode");
    }
  }

  read_battery_level();
  if (batteryLevel < 20.0) {    //If battery level below 20%
    turn_battery_low_led_on();  //Turn on the right LED to signal low battery charge level
  }

  print_wakeup_reason();  //print the reason for wake up from deep sleep 
  esp_sleep_enable_timer_wakeup(DEEP_SLEEP_IN_SEC * uS_TO_S_FACTOR);  //set the deep sleep duration
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);

//  start_access_point();
//  turn_on_setup();

  //Read all user defined values from memory and set the variables
  user_ssid = read_String(0);
  user_pw = read_String(user_ssid.length() + 1);  //+1: include the terminating zero character
  device_id = read_String(user_ssid.length() + user_pw.length() + 2);
  device_token = read_String(user_ssid.length() + user_pw.length() + device_id.length() + 3);
  longitude = read_String(user_ssid.length() + user_pw.length() + device_id.length() + device_token.length() + 4);
  latitude = read_String(user_ssid.length() + user_pw.length() + device_id.length() + device_token.length() + longitude.length() + 5);
}
