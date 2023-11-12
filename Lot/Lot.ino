/*
CS3237 AY2324S1
CARPARK LOT SENSOR
Written by: Sean Wong
*/

/*
I first tried pins 12, 13 but it did not work at all,
after alot of debugging i realised that I need to use RTC enabled GPIO pins

The Ultrasonic sensor works on 5V rail, we need to be careful
The LED probably cannot handle 5V
I have my breakout board set to 5V, might be harmful to certain devices

Ultrasonic Sensor: HC-SR04
VCC > +5V
GND > GND
TRIG > 25
ECHO > 26

LED
RED LED > 32
GREEN LED > 33
*/

// CONFIGURATION
#include "config.h"
#include <WiFi.h>

String strHostname = "ESP-LOT" + String(intLotNum);
String strLastWillMessage = strHostname + " gon be offline";
String ParkEventTopic = "event/lot/park/" + String(intLotNum);
String LeaveEventTopic = "event/lot/leave/" + String(intLotNum);
String lotAliveTopic = "status/carpark/lot";
// Time ESP32 will go to sleep (in seconds)
#define TIME_TO_SLEEP 10

// For deepsleep
#define uS_TO_S_FACTOR 1000000ULL  // Conversion factor for micro seconds to seconds

// Ultrasonic Sensor Setup
#define SOUND_SPEED 0.034
const int trigPin = 25; //Trigger is pin 25
const int echoPin = 26; //Echo is pin 26
long duration;
float distanceCm;
unsigned long lastEventMillis = 0; // to store the last time we got a nearby event

// LED Setup
#define RED_LED 32
#define GREEN_LED 33

// MQTT Setup
#include <ESP32MQTTClient.h>
ESP32MQTTClient mqttClient;

// Preseving of state of car detected across sleep cycles
RTC_DATA_ATTR bool boolCarDetected = false;

void setup() {
  Serial.begin(115200);
  
  // SET LEDs as output pin
  pinMode(RED_LED,OUTPUT);
  pinMode(GREEN_LED,OUTPUT);

  // Wifi connection
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  WiFi.setHostname(strHostname.c_str());
  delay(500);

  // Initialize and configure MQTT
  mqttClient.enableDebuggingMessages();
  mqttClient.setURI(server);
  mqttClient.enableLastWillMessage("lwt", strLastWillMessage.c_str());
  mqttClient.setKeepAlive(30);
  mqttClient.loopStart();
  while(!mqttClient.isConnected()) {
    delay(500);
    Serial.println("Waiting for MQTT Connection");
  }
  Serial.println("MQTT Connected!");

  // Initialise Ultrasonic Sensor
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  float currentDist = readDistanceCm();
  Serial.println("Current distance sensor reading: " + String(currentDist) + "cm");

  // Check reset reason
  // We use this to initialise for things that we need on the first boot
  esp_reset_reason_t reset_reason;
  reset_reason = esp_reset_reason();
  if (reset_reason == ESP_RST_POWERON) {
    // Code here will only run after a power-on reset (not after waking up from deep sleep)
    // If first boot already have car (power loss maybe)
    // Then it will publish a "1" and also set the LED to red first
    Serial.println("This is a power-on reset (first time code execution).");
    boolCarDetected = (currentDist <= fltDetectionDistanceCm);
    setLED();
    delay(2000);
    mqttClient.publish(lotAliveTopic, "Lot 1 is operational!", 0, false);
    delay(100);
    Serial.println("Alive MQTT message sent!");
    sendStatus();
    delay(100);
  } else if (reset_reason == ESP_RST_DEEPSLEEP) {
    // This route can be deleted later
    Serial.println("Woken up from deep sleep.");
  }

  bool currentCarStatus = (currentDist <= fltDetectionDistanceCm);

  if (boolCarDetected != currentCarStatus) {
    boolCarDetected = currentCarStatus;
    gpio_hold_dis((gpio_num_t)RED_LED);
    gpio_hold_dis((gpio_num_t)GREEN_LED);
    setLED();
    sendStatus();
    delay(100);
  }

  gpio_hold_en((gpio_num_t)RED_LED);
  gpio_hold_en((gpio_num_t)GREEN_LED);
  gpio_deep_sleep_hold_en();

  // Clean up before sleeping
  WiFi.disconnect(true);
  Serial.println("Disconnected from WIFI");
  Serial.println("Going to sleep now");
  Serial.flush();
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void loop() {
  // Will not run at all
}

void onConnectionEstablishedCallback(esp_mqtt_client_handle_t client) {
}

esp_err_t handleMQTT(esp_mqtt_event_handle_t event) {
  mqttClient.onEventCallback(event);
  return ESP_OK;
}

float readDistanceCm() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  return duration * SOUND_SPEED/2;
}

void setLED() {
  if (!boolCarDetected) {
    // Car NOT detected
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    Serial.println("CAR NOT DETECTED");
  } else {
    // Car detected
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    Serial.println("CAR DETECTED");
  }
}

void sendStatus() {
  if (boolCarDetected) {
    delay(3000);
    // Got car
    mqttClient.publish(ParkEventTopic, "Car Parked at Lot " + String(intLotNum), 0, false);
  } else {
    mqttClient.publish(LeaveEventTopic, "Car Left at Lot " + String(intLotNum), 0, false);
  }
}