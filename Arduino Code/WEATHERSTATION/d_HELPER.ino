//**HELPER FUNCTIONS**

/**
 * Writes a String in EEPROM memory starting at a defined address.
 *
 * @param  add  the address at which the first character of the string should be written
 * @param  data the String to be written to EEPROM memory
 */
void write_String(char add, String data) {
  int _size = data.length();
  int i;
  for (i = 0; i < _size; i++)
  {
    EEPROM.write(add + i, data[i]);
  }
  EEPROM.write(add + _size, '\0'); //Add termination null character for String Data
  EEPROM.commit();
}

/**
 * Reads a String from EEPROM memory starting at a defined address.
 * Reads until a terminating null character.
 *
 * @param  add  the address at which to start reading
 */
String read_String(char add) {
  int i;
  char data[200]; //Max size of EEPROM
  int len = 0;
  unsigned char k;
  k = EEPROM.read(add);
  //  Serial.println((char)k);
  while (k != '\0' && len < 199) //Read until null character
  {
    k = EEPROM.read(add + len);
    data[len] = k;
    len++;
  }
  data[len] = '\0';
  return String(data);
}

/**
 * Only for debug purposes: Prints the reason why the ESP woke up from sleep
 */
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0      : Serial.println("Wakeup caused by external signal using RTC_IO");          break;
    case ESP_SLEEP_WAKEUP_EXT1      : Serial.println("Wakeup caused by external signal using RTC_CNTL");        break;
    case ESP_SLEEP_WAKEUP_TIMER     : Serial.println("Wakeup caused by timer");                                 break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD  : Serial.println("Wakeup caused by touchpad");                              break;
    case ESP_SLEEP_WAKEUP_ULP       : Serial.println("Wakeup caused by ULP program");                           break;
    default                         : Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}
