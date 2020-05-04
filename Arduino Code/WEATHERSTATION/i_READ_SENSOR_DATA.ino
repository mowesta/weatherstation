//**FUNCTIONS FOR READING ALL DATA SOURCES AND STORING THE DATA**

/**
 * Reads the current battery charge level using one of the two ADCs integrated 
 * into the ESP32. The measured analog value is then converted to a percantage value.
 * 
 * The ESP32 ADC inputs can read voltage levels between 0V and 3.3V. When measuring, 
 * the voltage level on the ADC pin is converted to a value between 0 and 4095. 
 * That's because the ADC pins on the ESP32 mostly have 12-bit resolution (2^12 = 4096).
 * Because we are interested in the battery charge status in percent, the measured value
 * needs to be converted into a percantage value.
 * 
 * 4.2V -> 3.3V
 * 1V -> 3.3V/4.2V
 * 
 * 3.3V -> 4095
 * 1V -> 4095/3.3V
 * 
 * 4.2V -> 4095 (Overcharge Detection Voltage)
 * 3V -> 2925   (Over-discharge release voltage)
 * 2.4V -> 2340 (Lowest discharge voltage, protected by TP4056 over-discharge protection)
 * 
 * map() function uses integer math => not precise enough
 */
void read_battery_level() {
  float reading = 0; 
  for(int i = 0; i < 5; ++i) {
    reading += analogRead(BATTERY_LEVEL_PIN);
  }
  reading /= 5;
  batteryLevelVoltage = reading * 3.307f / 4095.0f * BATTERY_MAX_VOLTAGE / 3.3f;   //calculate the current voltage level
  batteryLevel = (reading - 2340.0f) * (100.0f - 0.0f) / (4095.0f - 2340.0f) + 0.0f;    //calculate a percentage value
  Serial.println("Battery reading: " + String(reading));
  Serial.println("Battery voltage: " + String(batteryLevelVoltage) + "V");
  Serial.println("Battery level: " + String(batteryLevel) + "%");
}

/**
 * Reads the current intensity of the sun shining onto the solar panel(s) using one of the two ADCs integrated 
 * into the ESP32. The measured analog value is then converted to a percantage value.
 */
void read_sunlight_strength() {
  float reading = 0; 
  for(int i = 0; i < 5; ++i) {
    reading += analogRead(SUNLIGHT_STRENGTH_PIN);
  }
  reading /= 5;
  sunlightStrengthVoltage = reading * 3.45f / 4095.0f * SOLAR_MAX_VOLTAGE / 3.3f;   //calculate the current voltage level
  sunlightStrength = reading / 4095.0f * 100;
  Serial.println("Solar panel reading: " + String(reading));
  Serial.println("Solar panel voltage: " + String(sunlightStrengthVoltage) + "V");
  Serial.println("Sunlight Strength: " + String(sunlightStrength) + "%");
}

/**
 * Reads the environmental data from up to two BME280 sensors connected to the weatherstation.
 * It measures temperature, humidity, pressure and altitude.
 * 
 * If there is more than one sensor connected, the I2C adresses of those need to be different.
 * This can be accomplished by chosing a different combination of solder jumpers soldered together.
 * For more information, please confer https://lastminuteengineers.com/bme280-arduino-tutorial/.
 *
 * @param  sensorId  the ID of the sensor connected to the weatherstation
 */
void read_environmental_sensor(int sensorId) {
  Serial.println("Measuring data...");
  /* 0-2 Device with L2C address 0x76 {tmp, hum, pres, altitude} */
  /* 3-5 Device with L2C address 0x77 {tmp, hum, pres, altitude} */

  //Take the average of multiple separate measurements
  short max = 5;
  for (short i = 0; i < max; ++i) {
    measurements[sensorId][1] += bme[sensorId].readTemperature();
    measurements[sensorId][2] += bme[sensorId].readHumidity();
    measurements[sensorId][3] += bme[sensorId].readPressure();
    measurements[sensorId][4] += bme[sensorId].readAltitude(SEALEVELPRESSURE_HPA);
  }
  measurements[sensorId][1] /= max;
  measurements[sensorId][2] /= (max * 100.0f);
  measurements[sensorId][3] /= (max * 100.0f);
  measurements[sensorId][4] /= max;

  /* Serial output vor debuggin reasons */
  Serial.println("SENSOR - " + String(sensorId));
  Serial.print("Temperature = ");
  Serial.print(measurements[sensorId][1]);
  Serial.println(" *C");

  Serial.print("Humidity = ");
  Serial.print(measurements[sensorId][2] * 100.0f);
  Serial.println(" %");

  Serial.print("Pressure = ");
  Serial.print(measurements[sensorId][3]);
  Serial.println(" hPa");

  Serial.print("Altitude = ");
  Serial.print(measurements[sensorId][4]);
  Serial.println(" m");

  Serial.println();
}

/**
 * Initialized the I2C interfaces of all BME280 environmental sensors connected to the weatherstation 
 * and performs a measurement on all sucessfully initialized sensors.
 */
void activate_read_environmental_sensors() {
  numberOfSensors = 0;

  Wire.begin(); //I2C bus

  if(bme[0].begin(0x76)) {  //Initialized the first I2C interface with the given address
    numberOfSensors++;
    Serial.println("I2C device found at address 0x76");
    measurements[0][0] = 1;
    read_environmental_sensor(0);
  }

  if(bme[1].begin(0x77)) {  //Initialized the second I2C interface with the given address
    numberOfSensors++;
    Serial.println("I2C device found at address 0x77");
    measurements[1][0] = 1;
    read_environmental_sensor(1);
  }

  if (numberOfSensors == 0) {
    Serial.println("No I2C device found on I2C bus");
  }
}
