//**LIBRARIES, CONSTANTS, VARIABLES, PINS, FUNCTION PROTOTYPES**

//Weatherstation running on an ESP32 developed for the Master Project "Crowd Sourcing of Temperature Data" 
//during the WS19/20 at the University of Duisburg-Essen: https://www.mowesta.com/

 /*  ___   ___    _______   __   _   __   _____     ____    ____     _
    ( (\\_//) )  (   _   )  \ \_/ \_/ /   ) ___)   /  _/   (_  _)   / \
     ) )\_/( (    ) )_( (    \   _   /   (  ___)   \_  \    (  )   / _ \
    (_(     )_)  (_______)    \_/ \_/     )____)   /___/    (__)  /_/ \_\
 */

//Include libraries
#include <WiFi.h>
#include <WiFiClient.h>
#include <EEPROM.h>
#include <Wire.h>
#include <HTTPClient.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_BME280.h"
#include "ArduinoJson.h"

#define VERSION 1
//#define MODE_TEST_SENSORS 1

//Defining various constants
#define BAUDRATE 115200
#define AP_PORT 8080

#ifdef MODE_TEST_SENSORS
  #define DEEP_SLEEP_IN_SEC 3     //Publish measurement data every 3 seconds
#else
  #define DEEP_SLEEP_IN_SEC 600   //Publish measurement data every 10 minutes
#endif

#define EEPROM_SIZE 512
#define SEALEVELPRESSURE_HPA (1013.25)  //Const needed to calculate height
#define uS_TO_S_FACTOR 1000000  //Factor to convert uS to S
#define JSON_BUFFER_SIZE 1000
#define MAX_SENSORS 2
#define DEBOUNCE_DELAY 30       //Debounce time for a reset butto press
#define RESET_LONG_PRESS_DURATION_MS 4000
#define BATTERY_MAX_VOLTAGE 4.2
#define SOLAR_MAX_VOLTAGE 6.0

//Defining Pins
#define RESET_PIN 0
#define BATTERY_LEVEL_PIN 32
#define SUNLIGHT_STRENGTH_PIN 33
#define RED_LED_PIN 18
#define GREEN_LED_PIN 17
#define BLUE_LED_PIN 16

//Credentials for the Access Point
const char *ssid = "MoWeSta - WiFi";
const char *password = "mowesta123456wifi";

//API-URL where measurements will be uploaded to
const char *backendURL = "https://www.mowesta.com/api/devices/current/measurements/now";

//Access point config
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

//Variables set to data values received by the client
String user_ssid;
String user_pw;
String device_id;
String device_token;
String longitude;
String latitude;

//Create sensor objects
Adafruit_BME280 bme[MAX_SENSORS]; //BME280 environmental sensor
short numberOfSensors = 0;
double measurements[MAX_SENSORS][5];
float batteryLevel;
float batteryLevelVoltage;
float sunlightStrength;
float sunlightStrengthVoltage;

bool setupOn = false;
bool rebootRequested = false;
bool setupOnLedOn = false;
bool batteryLowLedOn = false;
bool wifiPendingLedOn = false;
bool errorIndicationLedOn = false;
bool dataUploadIndicationLedOn = false;
int lastResetButtonState;
unsigned long lastDebounceTime = 0;  // the last time the reset pin was toggled

//Create hardware timer object
hw_timer_t *wifiPendingLedTimer = NULL;
hw_timer_t *setupModeOnLedTimer = NULL;
//Create task handle
TaskHandle_t Task1;

//RTC_DATA_ATTR int bootCount = 0;  //Variable stored in RTC  memory

//Declaring a button struct for the reset button
struct Button {
  const uint8_t PIN;
  bool pressed;
  bool longPressed;
  long timePressed;
  long longPressTime;
};
Button resetButton = {RESET_PIN, false, false, 0, RESET_LONG_PRESS_DURATION_MS};
WiFiServer accessPoint(AP_PORT);  //Create WiFiServer object which will be out access point

//Function prototypes
void IRAM_ATTR on_wifi_pending_led_timer();
void IRAM_ATTR on_setup_mode_on_led_timer();
void IRAM_ATTR on_reset_button_state_changed();
void detect_handle_reset_button_press();
void start_access_point();
bool connect_to_wifi();
void handle_client_connection(WiFiClient *client);
bool handle_client_data_json(String data);
void turn_off_setup();
void turn_on_setup();
bool is_setup_on();
void measure_upload_data();
void read_battery_level();
void read_sunlight_strength();
void activate_read_environmental_sensors();
void read_environmental_sensor(int sensorId);
void post_data_to_server(const char *URL);
void turn_setup_led_on();
void turn_battery_low_led_on();
void turn_wifi_pending_led_on();
void turn_led_off();
void set_led_pinout(int red, int green, int blue, bool blink);
void write_String(char add, String data);
String read_String(char add);
void print_wakeup_reason();
