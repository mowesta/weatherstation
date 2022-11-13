#define ARDUINOJSON_USE_LONG_LONG 1

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <EEPROM.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_BME280.h"
#include "ArduinoJson.h"

// Hardware setup 
#define VOLTAGE_PIN A0
#define VOLTAGE_MULTIPLIER 11
#define VOLTAGE_DIVIDER 255.0
#define EEPROM_SIZE 512
#define BUTTON_OUT D7
#define BUTTON_PIN D5

//Credentials for the Access Point
#define WLAN_AP_SSID "MoWeSta - WiFi"
#define WLAN_AP_PASSWORD "mowesta123456wifi"

// API-URL where measurements will be uploaded to
#define MOWESTA_UPLOAD_URL "https://www.mowesta.com/api/devices/current/measurements/now"
// #define MOWESTA_UPLOAD_URL "https://beta.mowesta.com/api/devices/current/measurements/now"
#define MOWESTA_SOFTWARE_VERSION 2

// memory for json document
#define JSON_BUFFER_SIZE 1000

// deep sleep period between measurements (us = 1e-6)
#define DEEP_SLEEP_PERIOD 600000000
// timeout to wait for the wifi connection in ms
#define TIMEOUT_WIFI 20000
// timeout to wait for the ntp sync in ms
#define TIMEOUT_NTP 20000
// timeout to wait for the setup data in sec
#define TIMEOUT_READ 40

// Let's encrypt root cert (valid until June 4, 2035)
const char certificate_anchors[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";
X509List certificates(certificate_anchors);

// Access point config
IPAddress ip_address_local(192, 168, 4, 1);
IPAddress ip_address_gateway(192, 168, 4, 1);
IPAddress ip_subnet_mask(255, 255, 255, 0);
WiFiServer wifi_server(8080);

// Variables set to data values received from the app
String* setting_wlan_ssid;
String* setting_wlan_password;
String* setting_device_id;
String* setting_device_token;
String* setting_longitude;
String* setting_latitude;

// Flag to store mode
bool mode_setup = false;

// Store boot time for timeout
long time_boot;

// Sensor to gather measurements
Adafruit_BME280 sensor_bme;

// eeprom reading and writing
void eeprom_write_string(int* add, String* data);
String* eeprom_read_string(int* add);

// settings reading, writing, printing and validation
void settings_read();
void settings_write();
void settings_print();
bool settings_valid();


// device setup via mobile app or netcat
bool setup_process_connection(WiFiClient*);
bool setup_process_data(String);

// measurement and upload
void measurement_capture();

void setup() {
  //Open serial port to display data and set baudrate
  Serial.begin(115200); 
  // Wait for serial port to be initialized
  while (!Serial);   
  Serial.println();

  // power the button
  pinMode(BUTTON_OUT, OUTPUT);
  digitalWrite(BUTTON_OUT, LOW);
  
  // Initialize EEPROM with complete size
  // Configure the button to enter setup
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // Configure the LED to indicate setup
  pinMode(LED_BUILTIN, OUTPUT);
  // Read the settings from eeprom
  EEPROM.begin(EEPROM_SIZE); 
  settings_read();
  // Check if button is pressed to enter setup
  mode_setup = (digitalRead(BUTTON_PIN) == LOW);
  time_boot = millis();
  if (mode_setup) {
    digitalWrite(LED_BUILTIN, LOW);    
    Serial.println("Entering setup mode.");
    WiFi.softAPConfig(ip_address_local, ip_address_gateway, ip_subnet_mask);
    WiFi.softAP(WLAN_AP_SSID, WLAN_AP_PASSWORD);  
    IPAddress ip_address = WiFi.softAPIP();
    Serial.print("IP address is: '");
    Serial.print(ip_address);
    Serial.println("'");
    wifi_server.begin();    
  } else if (settings_valid()){
    digitalWrite(LED_BUILTIN, HIGH);    
    Serial.print("Connecting to WiFi network '");
    Serial.print(*setting_wlan_ssid);
    Serial.println("'");
    WiFi.begin(setting_wlan_ssid->c_str(), setting_wlan_password->c_str());
  } else {
    Serial.println("Invalid settings, setup required.");
    settings_print();
  }
}

void loop() {
  if (mode_setup) {
    // setup mode, keeps handling incoming clients until successful setup
    WiFiClient client = wifi_server.available();
    if (client) {
      digitalWrite(LED_BUILTIN, HIGH);    
      if (setup_process_connection(&client)) {
        WiFi.softAPdisconnect(true);  
        delay(5000);
        ESP.restart();      
      } else {
        digitalWrite(LED_BUILTIN, LOW);            
      }
    }  
  } else if (settings_valid()) {
    // measurement mode, captures and uploads a measurement when wifi is up
    if (WiFi.status() == WL_CONNECTED) {
      if (sensor_bme.begin(0x76)) {
          measurement_capture();
      } else {
        Serial.println("Failed to activate sensor.");
      }
      WiFi.disconnect();
      delay(1000);
      Serial.end();
      ESP.deepSleep(DEEP_SLEEP_PERIOD);
    } else {
      // check for excessive waiting time
      long time_now = millis();
      long time_delta = (time_now > time_boot) ? (time_now - time_boot) : time_boot - time_now;
      if (time_delta > TIMEOUT_WIFI) {
        Serial.println("Failed to activate wifi.");
        WiFi.disconnect();
        delay(1000);
        Serial.end();
        ESP.deepSleep(DEEP_SLEEP_PERIOD);
      }
      delay(500);
    }
  } else {
    // blink to indicate missing config
    for(int i = 0; i < 10; i++){
      digitalWrite(LED_BUILTIN, LOW);   
      delay(50);                       
      digitalWrite(LED_BUILTIN, HIGH);    
      delay(50);           
    } 
    Serial.end();
    ESP.deepSleep(DEEP_SLEEP_PERIOD);
  }
}



/**
 * Writes a String in EEPROM memory starting at a defined address.
 *
 * @param add  In/out parameter with the address.
 * @param data The string to write.
 */
void eeprom_write_string(int* add, String* data) {
  int s = data->length();
  if (*add + s + 1 < EEPROM_SIZE) {
    for (int i = 0; i < s; i++) {
      EEPROM.write(*add + i, (*data)[i]);
    }
    EEPROM.write(*add + s, '\0');
  }
  *add += s + 1;
}

/**
 * Reads a String from EEPROM memory starting at a defined address.
 * Reads until a terminating null character.
 *
 * @param add In/out parameter with the address.
 * @return A pointer to the string.
 */
String* eeprom_read_string(int* add) {
  char data[EEPROM_SIZE + 1];
  int limit = EEPROM_SIZE - *add;
  limit = limit < 0 ? 0 : limit;
  int len = 0;
  unsigned char k;
  if (limit > 0) {
    k = EEPROM.read(*add);
    while (k != '\0' && len < limit)
    {
      k = EEPROM.read(*add + len);
      data[len] = k;
      len++;
    }
  }
  data[len] = '\0';
  *add += len;
  return new String(data);
}

/**
 * Reads the settings from the eeprom to memory.
 */
void settings_read() {
  int address = 0;
  setting_wlan_ssid = eeprom_read_string(&address);
  setting_wlan_password  = eeprom_read_string(&address);
  setting_device_id = eeprom_read_string(&address);
  setting_device_token = eeprom_read_string(&address);
  setting_longitude = eeprom_read_string(&address);
  setting_latitude = eeprom_read_string(&address);
}

/**
 * Writes the settings from memory to the eeprom.
 */
void settings_write() {
  int address = 0;
  eeprom_write_string(&address, setting_wlan_ssid);
  eeprom_write_string(&address, setting_wlan_password);
  eeprom_write_string(&address, setting_device_id);
  eeprom_write_string(&address, setting_device_token);
  eeprom_write_string(&address, setting_longitude);
  eeprom_write_string(&address, setting_latitude);
  EEPROM.commit();
}

/**
 * Prints the settings onto the serial console.
 */
void settings_print() {
  Serial.print("WLAN SSID: ");
  Serial.println(*setting_wlan_ssid);
  Serial.print("WLAN Password: ");
  Serial.println(*setting_wlan_password);
  Serial.print("Device ID: ");
  Serial.println(*setting_device_id);
  Serial.print("Device Token: ");
  Serial.println(*setting_device_token);
  Serial.print("Longitude: ");
  Serial.println(*setting_longitude);
  Serial.print("Latitude: ");
  Serial.println(*setting_latitude);
}

/**
 * Determines whether the settings are valid. This is
 * done by making sure that every string has a non-zero length.
 * 
 * @return True if the settings are valid, false otherwise.
 */
bool settings_valid() {
    return setting_wlan_ssid->length() > 0 &&
      setting_wlan_password->length() > 0 &&
      setting_device_id->length() > 0 &&
      setting_device_token->length() > 0 &&
      setting_longitude->length() > 0 &&    
      setting_latitude->length() > 0;
}

/**
 * Parses the client data and stores the relevant information in flash memory. 
 * It is assumed that the client data is in json format. 
 *
 * @param data The data received from the client.
 * @return True if the data seems ok and has been stored to flash, false otherwise.
 */
bool setup_process_data(String data) {
  StaticJsonDocument<JSON_BUFFER_SIZE> doc;  //Memory pool
  Serial.print("Received settings: ");
  Serial.println(data);
  DeserializationError error = deserializeJson(doc, data);
  if (error) {
    Serial.print("Deserialization failed with code ");
    Serial.println(error.c_str());
    return false;
  } else {
    delete setting_wlan_ssid;
    delete setting_wlan_password;
    delete setting_device_id;
    delete setting_device_token;
    delete setting_longitude;
    delete setting_latitude;
    setting_wlan_ssid = new String(doc["ssid"]);
    setting_wlan_password = new String(doc["password"]);
    setting_device_id = new String(doc["id"]);
    setting_device_token = new String(doc["token"]);
    setting_longitude = new String(doc["longitude"]);
    setting_latitude = new String(doc["latitude"]);
    settings_print();
    if (settings_valid()) {
      Serial.println("Storing valid settings.");
      settings_write();
      return true;
    } else {
      Serial.println("Ignoring invalid settings.");
      return false;
    }    
  }
}


/**
 * Performs the setup data exchange with the app and returns
 * whether the arduino has successfully received a new configuration.
 *
 * @param client The wifi client that represents the connection.
 * @return True if the setup data has been retrieved successfully.
 */
bool setup_process_connection(WiFiClient *client) {
  Serial.println("Client connection established.");           
  String data = "";
  bool didRead = true;
  client->setTimeout(TIMEOUT_READ);                        
  while (client->connected() && didRead) {       
    String chunk = client->readStringUntil('\n');
    didRead = chunk.length() > 0; // if line is empty, break immediately
    data += chunk;
  }
  if (data.length() > 0) {
    if(setup_process_data(data)) {
      client->println("Success\n");   // Notify client
      client->flush();
      // Wait for client to close connection, since we will 
      // be disabling the accesspoint directly afterwards
      while (client->connected()); 
      client->stop();
      Serial.println("Success, client disconnected.");
      return true;
    } else {
      client->println("Error\n");     //Notify client
      client->flush();
      client->stop();
      Serial.println("Error, client disconnected.");
      return false;
    }
  } else {
    client->println("Timeout\n");     //Notify client
    client->flush();
    client->stop();
    Serial.println("Timeout, client disconnected.");
    return false;
  }
}


/**
 * Synchronizes the time with an ntp server, captures a measurement,
 * generates a json document and uploads it to the mowesta backend.
 */
void measurement_capture() {
  WiFiClientSecure wifi;
  HTTPClient http;
  StaticJsonDocument<JSON_BUFFER_SIZE> doc;

  // set trusted root certificates and perform ntp time sync for https
  configTime(0, 0, "time2-1.uni-duisburg-essen.de", "time2-3.uni-duisburg-essen.de", "pool.ntp.org");
  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  // check that time is after January 1st, 2021
  int max_wait = TIMEOUT_NTP;
  while (now < 1606694400) {
    if (max_wait < 0) {
      Serial.println("Timeout");
      return;
    }
    delay(500);
    max_wait -= 500;
    now = time(nullptr);
  }
  Serial.println(now);
  wifi.setTrustAnchors(&certificates);
 
  // create json document for upload
  doc["id"] = 0;
  doc["creationTime"] = 0;
  doc["modificationTime"] = 0;
  doc["deleted"] = "false";
  JsonObject coordinate = doc.createNestedObject("coordinate");
  coordinate["latitude"] = setting_latitude->toDouble();
  coordinate["longitude"] = setting_longitude->toDouble();
  doc["time"] = now * 1000;           
  doc["device"] = atol(setting_device_id->c_str());        
  doc["version"] = MOWESTA_SOFTWARE_VERSION;
  doc["temperature"] = (char*)0;
  doc["pressure"] = (char*)0;
  doc["humidity"] = (char*)0;
  JsonObject data = doc.createNestedObject("data");
  data["SENSOR_TEMPERATURE"] = String(sensor_bme.readTemperature());
  data["SENSOR_PRESSURE"] = String(sensor_bme.readPressure() / 100);
  data["SENSOR_HUMIDITY"] = String(sensor_bme.readHumidity());
  data["BATTERY_VOLTAGE"] = String((analogRead(VOLTAGE_PIN) / VOLTAGE_DIVIDER) * VOLTAGE_MULTIPLIER);
  
  // print measurement
  serializeJsonPretty(doc, Serial); 
  Serial.println();

  // upload measurement
  String body;
  serializeJson(doc, body); 
  http.begin(wifi, MOWESTA_UPLOAD_URL);
  http.addHeader("Accept", "application/json");
  http.addHeader("Authorization", "Bearer " + *setting_device_token);
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(body); 
  if (code / 100 == 2) {
    Serial.println("Upload successful.");    
  } else {
    Serial.print("Upload failed: ");
    Serial.println(code);
  }  
  http.end();
}
