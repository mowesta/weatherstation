//**FUNCTIONS FOR UPLOADING THE GATHERED SENSOR DATA TO THE BACKEND**

/**
 * Build and send a HTTP POST packet for transmitting all measured data to the backend server.
 *
 * @param  URL  the URL to send data to
 */
void post_data_to_server(const char *URL) {
  Serial.println("Sending measurements to backend...");
  HTTPClient http;

  String temp, pres, humi = "";
  if (measurements[0][0] > 0) {
    temp = String(measurements[0][1]) + (measurements[1][0] ? "," : "");
    pres = String(measurements[0][3]) + (measurements[1][0] ? "," : "");
    humi = String(measurements[0][2]) + (measurements[1][0] ? "," : "");
  }
  if (measurements[1][0] > 0) {
    temp += String(measurements[1][1]);
    pres += String(measurements[1][3]);
    humi += String(measurements[1][2]);
  }

  StaticJsonDocument<JSON_BUFFER_SIZE> doc; //Allocates memory for the document

  /* Construct a collection of key-value pairs in JSON format to be send to the API function */
  //JsonArray elements = doc.createNestedArray("elements");
  //JsonObject data = doc.createNestedObject();
  doc["id"] = 0;
  doc["creationTime"] = 0;
  doc["modificationTime"] = 0;
  doc["deleted"] = "false";
  JsonObject coordinate = doc.createNestedObject("coordinate");
  coordinate["latitude"] = latitude.toDouble();
  coordinate["longitude"] = longitude.toDouble();
  doc["time"] = 0;           // Has to be 0 so that it can be set by the server
  doc["device"] = 0;         // Will be changed on the server side
  doc["version"] = VERSION;
  doc["temperature"] = (char*)0;
  doc["pressure"] = (char*)0;
  doc["humidity"] = (char*)0;
  JsonObject additionalData = doc.createNestedObject("data");
  additionalData["SENSOR_TEMPERATURE"] = temp;
  additionalData["SENSOR_PRESSURE"] = pres;
  additionalData["SENSOR_HUMIDITY"] = humi;
  additionalData["BATTERY_VOLTAGE"] = (batteryLevelVoltage > 0.0 ? String(batteryLevelVoltage) : "");
  additionalData["SOLAR_VOLTAGE"] = (sunlightStrengthVoltage > 0.0 ? String(sunlightStrengthVoltage) : "");

  serializeJsonPretty(doc, Serial); //Serialize the JSON object for printing
  Serial.println();

  String body;
  serializeJson(doc, body); //Serialize the JSON object for transmitting it via http

  /* Build the HTTP POST request packet */
  http.begin(URL);
  http.addHeader("accept", "application/json");
  http.addHeader("Authorization", "Bearer " + device_token);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(body); //Transmit the JSON string as the body of the request

  turn_led_off();

  if (httpResponseCode > 0) { //No error occured
    Serial.println(httpResponseCode + " " + http.getString());

    //Visually notify the user that no error occured
    unsigned long tempTime;
    for (int i=0; i < 2; i++) {
      tempTime = millis();
      data_upload_indication_led_on();
      while(millis() - tempTime < 200);
    }
  } else {  //An error occured
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);

    //Visually notify the user that an error occured
    unsigned long tempTime;
    for (int i=0; i < 6; i++) {
      tempTime = millis();
      error_indication_led_on();
      while(millis() - tempTime < 300);
    }
  }

  http.end();
}
