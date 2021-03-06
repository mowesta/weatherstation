//**FUNCTIONS FOR PROCESSING NEW CLIENT DATA**

/**
 * Reads all data from the client, stores it in a string 
 * and forwards it for further processing.
 *
 * @param  client  the client connected to the weatherstation
 */
void handle_client_connection(WiFiClient *client) {
  Serial.println("New Client connected.");           // print a message out the serial port
  String data = "";
  bool didRead = true;
  client->setTimeout(10); // increase timeout to 10 seconds so that user can copy-paste data into netcat
  while (client->connected() && didRead) {        // wait at most 10 seconds or until empty line
    String chunk = client->readStringUntil('\n'); // read next line
    didRead = chunk.length() > 0;                 // if line is empty, break immediately
    data += chunk;
  }
  if (data.length() > 0) {
    if(handle_client_data_json(data)) {  //Process the read data
      client->println("Success\n");   // Notify client
      client->flush();
      // Wait for client to close connection, since we will
      // be disabling the accesspoint directly afterwards
      while (client->connected());
      client->stop();
      Serial.println("Success, client disconnected.");
      WiFi.softAPdisconnect(true);  //Shut down AP
      timerAlarmDisable(setupModeOnLedTimer);
      turn_led_off();
      turn_off_setup();
      connect_to_wifi();  //Connect to WiFi using the newly entered credentials
    } else {
      client->println("Error\n");     //Notify client
      client->flush();
      client->stop();
      Serial.println("Error, client disconnected.");
      //Also visually notify the client that an error occured
      timerAlarmDisable(setupModeOnLedTimer);
      turn_led_off();
      unsigned long tempTime;
      for (int i=0; i < 6; i++) {
        tempTime = millis();
        error_indication_led_on();
        while(millis() - tempTime < 300);
      }
      timerAlarmEnable(setupModeOnLedTimer);
    }
  } else {
    client->println("Timeout\n");     //Notify client
    client->flush();
    client->stop();
    Serial.println("Timeout, client Disconnected.");
  }
}

/**
 * Parses the client data and stores the relevant information in flash memory. 
 * It is assumed that the client data is in json format. 
 *
 * @param  data  the data received from the client
 */
bool handle_client_data_json(String data) {
  StaticJsonDocument<JSON_BUFFER_SIZE> doc;  //Memory pool
  DeserializationError error = deserializeJson(doc, data);
  if (error) {
    Serial.print("DeserializeJson() failed with code ");
    Serial.println(error.c_str());
    return false;
  }

  const char * ssid = doc["ssid"];                //Get ssid
  const char * pw = doc["password"];              //Get pw
  const char * id = doc["id"];                    //Get id
  const char * token = doc["token"];              //Get token
  const char * longi = doc["longitude"];          //Get longitude
  const char * lati = doc["latitude"];            //Get latitude

  Serial.println("\n----- NEW DATA FROM CLIENT ----");
  Serial.println("SSID: " + String(ssid)); 
  Serial.println("PW: " + String(pw));
  Serial.println("ID: " + String(id));
  Serial.println("Token: " + String(token));
  Serial.println("Longitude: " + String(longi));
  Serial.println("Latitude: " + String(lati));

  size_t ssid_len = strlen(ssid);
  size_t pw_len = strlen(pw);
  size_t id_len = strlen(id);
  size_t token_len = strlen(token);
  size_t longi_len = strlen(longi);
  size_t lati_len = strlen(lati);

  if (ssid_len > 0 && pw_len > 0) {   //Only continue if the client has sent a SSID and PW
    write_String(0, ssid);            //Write SSID into flash memory
    write_String(ssid_len + 1, pw);   //Write password behind the SSID string into flash memory

    //Read WiFi SSID and PW from memory
    user_ssid = read_String(0);
    user_pw = read_String(user_ssid.length() + 1);

    if (id_len > 0) { //Process the device ID
      write_String(ssid_len + pw_len + 2, id);
      device_id = read_String(ssid_len + pw_len + 2);
    }

    if (token_len > 0) {  //Process the device token
      write_String(ssid_len + pw_len + id_len + 3, token);
      device_token = read_String(ssid_len + pw_len + id_len + 3);
    }

    if (longi_len > 0) {  //Process the longitude
      write_String(ssid_len + pw_len + id_len + token_len + 4, longi);
      longitude = read_String(ssid_len + pw_len + id_len + token_len + 4);
    }

    if (lati_len > 0) {   //Process the latitude
      write_String(ssid_len + pw_len + id_len + token_len + longi_len + 5, lati);
      latitude = read_String(ssid_len + pw_len + id_len + token_len + longi_len + 5);
    }

    return true;
  }

  return false;
}
