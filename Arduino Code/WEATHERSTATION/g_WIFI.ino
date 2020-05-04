//**FUNCTIONS RELATED TO WIFI and AP MANAGEMENT**

/**
 * Configures and actives the access point on the ESP to receive client connections.
 */
void start_access_point() {
  WiFi.disconnect();
  
  Serial.println("\nConfiguring access point...");

  Serial.println("Wait 100 ms for AP_START...");
  //Wait for 100 ms 
  unsigned long tempTime = millis();
  while(millis() - tempTime < 100);

  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  accessPoint.begin();
  Serial.println("Access point stated.");
}

/**
 * The weatherstion tries to connect to the client's WiFi network.
 * The network credentials were previously transmitted to the wheaterstion by the client.
 *
 * @param  ssid  the ssid of the client WiFi network
 * @param  pw  the password of the client WiFi network
 */
bool connect_to_wifi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Trying to connect to WiFi network '" + user_ssid + "'...");
    
    WiFi.begin(user_ssid.c_str(), user_pw.c_str());   //Connect to User WiFi network
  
    bool timerEnabled = false;
    unsigned long timeWifiPending = millis();
    while (WiFi.status() != WL_CONNECTED) {

      //handle_reset_button_press();    //Handle a reset button press if present
      if (is_setup_on()) {              //Leave WiFi loop when long reset button press is detected
        return false;
      }

      //Turn on WiFi pending LED only after the weatherstion 
      //has been trying to connect to the WiFi network for at least 5 secs
      if(millis() - timeWifiPending > 5000 && timerEnabled == false) {
        timerEnabled = true;
        timerAlarmEnable(wifiPendingLedTimer);
      }
  
      //Stop connecting to WiFi after 16 seconds of trying
      if(millis() - timeWifiPending > 15500) {
        return false;
      }

      delay(10);
    }
  
    if(timerEnabled) {  //Turn WiFi pending LED timer off and all LEDs off
      timerAlarmDisable(wifiPendingLedTimer);
      turn_led_off();
    }
  
    Serial.println("\nWiFi connected");
  }

  return true;
}
