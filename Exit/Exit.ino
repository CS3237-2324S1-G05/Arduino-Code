/*
CS3237 AY2324S1
EXIT GANTRY CODE
Done by: Sean Wong
*/

/*
Board: esp32doit-devkit-v1
Core: esp32
Libraries
- LiquidCrystal_I2C@1.1.4
- Wire@2.0.0
- ESP8266 and ESP32 OLED driver for SSD1306 displays@4.4.0
- ESP32Servo@1.1.0
- ESP32MQTTClient@0.1.0
- WiFi@2.0.0
*/

/*
Both the LCD, the Ultrasonic sensor and servo works on 5V rail, we need to be careful
I have my breakout board set to 5V, might be harmful to certain devices
OLED Works on 3.3V Rail

Ultrasonic Sensor: HC-SR04
VCC > +5V
GND > GND
TRIG > 25
ECHO > 26

Servo: SG90
Pin: 2

LCD: 1602 with I2C Bus
VCC > +5V
GND > GND
SDA > 21
SCL > 22

http://wiki.sunfounder.cc/index.php?title=I%C2%B2C_LCD1602
LCD is a 1602 LCD with I2C Bus, with potentiometer at the back to adjust contrast
Default address is 0X27
Library to interface with it is the LiquidCrystal_I2C library

OLED Shield
3v3 > +3.3V
GND > GND
D1 > 21 (SCL)
D2 > 22 (SDA)
*/

#include "config.h"
#include "oledimage.h"

// LCD Setup
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);

// OLED Setup
#include <Wire.h>
#include "SSD1306Wire.h"
SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_64_48);

// Ultrasonic Sensor Setup
#define SOUND_SPEED 0.034
const int trigPin = 25; //Trigger is pin 25
const int echoPin = 26; //Echo is pin 26
long duration;
float distanceCm;

// Servo Setup
#include <ESP32Servo.h>
int servoPin = 2;
int pos = 0;
Servo myservo;  // create servo object to control a servo

// MQTT Setup
#include <ESP32MQTTClient.h>
#include <WiFi.h>

ESP32MQTTClient mqttClient;
String strLastWillMessage = strHostname + " going offline";

// Carpark logic
String strCarPlate = "";
String strParkingFee = "0.00";
bool boolCarPlate = false;
bool boolParkingFee = false;
bool boolObjectDetected = false;
String strNumLotAvail = "";
bool boolInitPhase = true;
unsigned long longCarAtGantryTime = 0;
unsigned long longLastEventMillis = 0;

void setup() {
  Serial.begin(115200);

  initLCD();
  initOLED();
  oledDrawCourseImage();

  oledDrawProgressBar(0);

  lcdPrintConnectionDetails();
  oledDrawProgressBar(20);

  initWIFI();
  oledDrawProgressBar(40);

  initMQTT();
  oledDrawProgressBar(60);

  initUltraSonicSensor();
  oledDrawProgressBar(80);

  initServo();
  lcdPrintDefault();
  oledDrawProgressBar(100);
  oledDrawInitComplete();
  oledDrawCourseImage();

  if (strcmp(strNumLotAvail.c_str(), "") != 0) {
    //If there is something in the numLotAvail
    oledPrintAvailLots(strNumLotAvail);
  }
  boolInitPhase = false;
}

void loop() {
  if (!boolObjectDetected){
    checkNearbyEvent();
    delay(500);
  } else {
    if (!boolCarPlate || !boolParkingFee){
      Serial.println("Waiting for parking fee and carplate");
      lcdPrintCarDetected(); // This function presumably prints something relevant on an LCD.
      // Handle if it takes more than 30s to get any result, car get to exit for free
      Serial.println("Time elapsed" + String((millis() - longCarAtGantryTime)));
        if (longCarAtGantryTime == 0) {
          longCarAtGantryTime = millis(); // Mark the start when the car is at gantry
        } else if (millis() - longCarAtGantryTime > intGantryTimeOut) {
          // means car waited for more than the timeout time
          freeExit();
          clearState();
          lcdPrintDefault();
        }
      delay(1000); // Wait for a second before re-checking
    } else {
      lcdPrintCarplate();
      delay(2000);
      lcdPrintPrice();
      delay(3000);
      openGantry();
      checkCarGone();
      closeGantry();
      clearState();
      lcdPrintDefault();
    }
  }
}

void onConnectionEstablishedCallback(esp_mqtt_client_handle_t client) {
  if (mqttClient.isMyTurn(client)){
    // Carplate
    mqttClient.subscribe(exitCarplateTopic, [](const String &message){
      Serial.println(exitCarplateTopic + ": " + message);
      strCarPlate = message;
      boolCarPlate = true;
      Serial.println("Extracted plate number: " + strCarPlate);
    });
    // Parking Fees
    mqttClient.subscribe(exitParkingFeeTopic, [](const String &message){
      Serial.println(exitParkingFeeTopic + ": " + message);
      strParkingFee = message;
      boolParkingFee = true;
      Serial.println("Extracted parking fee: " + strParkingFee);
    });
    mqttClient.subscribe(exitNumLotsTopic, [](const String &message) {
      Serial.println(exitNumLotsTopic + ": " + message);
      Serial.println("Extracted available lots: " + message);
      if (!boolInitPhase) {
        oledPrintAvailLots(message);
      } else {
        strNumLotAvail = message;
      }
    });
  }
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

void lcdPrintInit() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  //column, row
  lcd.setCursor(1, 0);
  lcd.print("ESP32-EXIT");
  lcd.setCursor(1,1);
  lcd.print("HELLO THERE :D");
}

void lcdPrintDefault() {
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("CS3237 CARPARK");
  lcd.setCursor(2,1);
  lcd.print("EXIT GANTRY");
}

void lcdPrintCarDetected() {
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("PLS HOLD");
  lcd.setCursor(1, 1);
  lcd.print("CS3237 CARPARK");
}

void lcdPrintCarplate() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("CARPLATE:");
  lcd.print(strCarPlate);
  lcd.setCursor(0,1);
  lcd.print("HOLD FOR CHARGES");
}

void lcdPrintPrice() {
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print(strCarPlate);
  if (strcmp(strParkingFee.c_str(), "None") == 0) {
    lcd.setCursor(2,1);
    lcd.print("FREE PARKING!");
  } else {
    lcd.setCursor(1,1);
    lcd.print("$");
    lcd.print(strParkingFee);
    lcd.print(" Charged.");
  }
}

void lcdPrintDebug(String s) {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (s.length() <= 16) {
    lcd.print(s);
  } else {
    lcd.print(s.substring(0, 16));  // Print first 16 characters
    if (s.length() > 16) {
      lcd.setCursor(0, 1);  // Move cursor to the second line
      lcd.print(s.substring(16));  // Print the rest of the characters
    }
  }
}

void openGantry() {
  myservo.write(0);
}

void closeGantry() {
  myservo.write(90);
}

void clearState() {
  String strCarPlate = "";
  String strParkingFee = "0.00";
  boolCarPlate = false;
  boolParkingFee = false;
  boolObjectDetected = false;
  longCarAtGantryTime = 0;
  longLastEventMillis = 0;
}

void checkNearbyEvent() {
  float currentDist = readDistanceCm();
  Serial.println("Dist: " + String(currentDist) + "cm");
  if (currentDist <= fltDetectionDistanceCm) {
    if (longLastEventMillis == 0) {
      Serial.println("Something is nearby, setting current time");
      longLastEventMillis = millis();
    } else if (millis() - longLastEventMillis > intCarDetectionTime) {
      delay(1000);
      mqttClient.publish(exitEventTopic, "Object Detected", 0, false);
      Serial.println("Published nearby event to MQTT");
      boolObjectDetected = true;
    } else {
      // This happens when something nearby is detected but has not reached timeout
      Serial.println("Time elapsed" + String((millis() - longLastEventMillis)));
    }
  } else {
    longLastEventMillis = 0;
    Serial.println("Nothing nearby, reset last event time to 0");
  }
}

// void checkNearbyEvent2() {
//   float currentDist = readDistanceCm();
//   Serial.println("Dist: " + String(currentDist) + "cm");
//   if (currentDist <= fltDetectionDistanceCm && millis() - lastEventMillis >= 2000) {
//     // Add delay for camera to stabilize
//     delay(2000);
//     mqttClient.publish(exitEventTopic, "Object Detected", 0, false);
//     Serial.println("Published nearby event to MQTT");
//     lastEventMillis = millis();  // update the time we sent the message
//     boolObjectDetected = true;
//   }
// }

void checkCarGone() {
  float currentDist;
  // Wait until the car is detected at a distance greater than 10cm or until 5 seconds have passed
  unsigned long goneStartTime = 0;  // To track when the car first goes beyond the 10cm mark
  bool carGoneFor5Seconds = false;  // To check if car has been gone for 5 seconds
  
  while (!carGoneFor5Seconds) {
    currentDist = readDistanceCm();
    Serial.println("Waiting for car to go out of carpark");
    // Check if car is gone
    if (currentDist > fltDetectionDistanceCm) {
      if (goneStartTime == 0) {
        goneStartTime = millis();  // Mark the start time when the car first goes beyond the 10cm mark
      } else if (millis() - goneStartTime > intCarDetectionTime) {  // Check if 5 seconds have passed since the car first went beyond the 10cm mark
        carGoneFor5Seconds = true;
        Serial.println("CAR GONE");
      }
    } else {
      goneStartTime = 0;  // Reset the timer if the car comes back within the 10cm mark
    }
    delay(500);     
  }
}

void oledDrawProgressBar(int intProgressPercent) {
  // int progress = (intProgressPercent / 5) % 100;
  int progress = intProgressPercent;
  
  // Clear the display
  display.clear();
  
  // draw the progress bar
  display.drawProgressBar(0, 32, 60, 10, progress);

  // Draw word Progress
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(32, 0, "INIT Progress");
  display.drawString(32, 15, String(progress) + "%");
  display.display();
}

void oledDrawInitComplete() {
  display.clear();
  display.display();
  delay(200);
  oledDrawProgressBar(100);
  delay(200);
  display.clear();
  display.display();
  delay(200);
  oledDrawProgressBar(100);
  delay(200);
}

void oledPrintAvailLots(String strAvailLots) {
  display.clear();
  display.display();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  display.drawString(32, 0, "AVAIL LOTS");
  display.setFont(ArialMT_Plain_24);
  display.drawString(32, 20, strAvailLots);
  display.display();
}

void initLCD() {
  // Initialise LCD
  Serial.println("LCD TEST BEGIN");
  lcdPrintInit();
  delay(500);
  lcd.clear();
  Serial.println("LCD TEST END");
}

void initOLED() {
  // Setup OLED
  Serial.println("OLED TEST BEGIN");
  display.init();
  display.resetDisplay();
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setColor(WHITE);
  display.drawString(0, 0, "OLED TEST");
  display.display();
  delay(500);
  Serial.println("OLED TEST END");
}

void lcdPrintConnectionDetails() {
  // Print basic messages
  lcdPrintDebug("Hostname: " + strHostname);
  delay(200);
  lcdPrintDebug("SSID: " + String(ssid));
  delay(300);
  lcdPrintDebug("MQTT: " + String(server));
  delay(300);
}

void initWIFI() {
  // Wifi connection
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
    lcdPrintDebug("WIFI CONNECTING");
  }
  Serial.println("Connected to WiFi");
  WiFi.setHostname(strHostname.c_str());
  lcdPrintDebug("WIFI CONNECTED");
  delay(300);
}

void initMQTT() {
  // Initialize and configure MQTT
  mqttClient.enableDebuggingMessages();
  mqttClient.setURI(server);
  mqttClient.enableLastWillMessage("lwt", strLastWillMessage.c_str());
  mqttClient.setKeepAlive(30);
  mqttClient.loopStart();
  while(!mqttClient.isConnected()) {
    delay(500);
    Serial.println("Waiting for MQTT Connection");
    lcdPrintDebug("CONNECTING MQTT");
  }
  Serial.println("MQTT Connected!");
  mqttClient.publish(exitAliveTopic, "Exit Gantry is operational!", 0, false);
  Serial.println("Alive MQTT message sent!");
}

void initUltraSonicSensor() {
  // Initialise Ultrasonic Sensor
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  float currentDist = readDistanceCm();
  Serial.println("Current distance sensor reading: " + String(currentDist) + "cm");
  lcdPrintDebug("Dist: " + String(currentDist) + "cm");
  delay(200);
}

void initServo() {
  // Initialise Servo
  ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
	myservo.setPeriodHertz(50);
  myservo.attach(servoPin, 500, 2450);

  Serial.println("INIT: Gantry opened.");
  myservo.write(0);
  lcdPrintDebug("GANTRY OPEN");
  delay(500);
  Serial.println("INIT: Gantry closed.");
  myservo.write(90);
  lcdPrintDebug("GANTRY Closed");
  delay(500);
}

void oledDrawCourseImage() {
  display.clear();
  display.drawXbm(0, 0, 64, 48, cs3237image);
  display.display();
  delay(2000);
}

void freeExit() {
  lcdPrintFreeExit();
  delay(3000);
  openGantry();
  checkCarGone();
  closeGantry();
  clearState();
}

void lcdPrintFreeExit() {
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("THANK YOU");
  lcd.setCursor(3,1);
  lcd.print("FREE EXIT!");
}