//**LOOP FUNCTION**

void loop() {

  if (is_setup_on()) {  //When setup mode is on wait for incoming connections
    //check for connected clients and if available, process further 
    WiFiClient client = accessPoint.available();
    if (client) {
      handle_client_connection(&client);
    }
  } else {  //Setup mode is off

    #ifdef MODE_TEST_SENSORS

      activate_read_environmental_sensors();  //Activate and read all environmental sensors
      read_sunlight_strength();               //Read the sunlight strength

      //Disconnect WiFi and go to sleep
      Serial.println("\nSLEEP");
      delay(10);              //Wait for the WiFi to successfully disconnect
      esp_deep_sleep_start(); //deep sleep mode

    #else
    
      if(resetButton.pressed == false) {
        bool wifiState = connect_to_wifi();  //Try to connect to WiFi
        if (resetButton.pressed == false && !is_setup_on()) {
            if(wifiState) {       //Check for successfully connected WiFi
              activate_read_environmental_sensors();  //Activate and read all environmental sensors
              read_sunlight_strength();               //Read the sunlight strength
              post_data_to_server(backendURL);        //Post all data to the backend
            }
      
          //Disconnect WiFi and go to sleep
          Serial.println("\nSLEEP");
          WiFi.disconnect(true);  //Disconnect WiFi
          delay(10);              //Wait for the WiFi to successfully disconnect
  
          //During deep sleep only RTC controller, RTC peripherals (including ULP co-processor)
          //and RTC memories (slow and fast) remain powered on.
          //The ULP co-processor does sensor measurements and can wake up the main system based on the sensor data.
          esp_deep_sleep_start(); //deep sleep mode
        }
      }

    #endif
  }

  //Wait for 50 ms 
  unsigned long tempTime = millis();
  while(millis() - tempTime < 50);
  //delay(50); //delay() is a blocking function and therefore prevents proper multitasking
}
