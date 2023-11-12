/*
CS3237 AY2324S1 for ESP Camera
ESP-CAM CODE
Written by: Sean Wong
 */
// ENTRANCE, EXIT, LOT1, LOT0, HUMAN
#define ENTRANCE

#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32MQTTClient.h>
#include "config.h"

ESP32MQTTClient mqttClient;
const String strLastWillMessage = strHostname + " going offline";

// Camera model
#define CAMERA_MODEL_AI_THINKER // Has to be before #include "camera_pins.h"
#include "camera_pins.h"

void startCameraServer(); // Forward declaration for starting the camera server

void setup() {
  Serial.begin(115200);

  // Camera config
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_UXGA;
  // config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  // 4 is best already, below 4 = better quality but not enough buffer
  config.jpeg_quality = 4;
  config.fb_count = 2;

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Set some camera parameters for better imaging
  sensor_t * s = esp_camera_sensor_get();
  s->set_brightness(s, 1);     // -2 to 2
  s->set_contrast(s, 2);       // -2 to 2
  s->set_saturation(s, 0);     // -2 to 2

  // To account for the off by one error
  camera_fb_t * fb = esp_camera_fb_get();
  delay(1000);
  esp_camera_fb_return(fb);


  // Connect to wifi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected: ");
  Serial.println(WiFi.localIP());

  // Initialize and configure MQTT
  mqttClient.enableDebuggingMessages();
  mqttClient.setURI(server);
  mqttClient.enableLastWillMessage("lwt", strLastWillMessage.c_str());
  mqttClient.setKeepAlive(30);
  mqttClient.loopStart();
  while (!mqttClient.isConnected()) {
    delay(500);
    Serial.println("Waiting for MQTT Connection");
  }
  Serial.println("MQTT Connected!");

}

void loop() {
  // Not rly important in this case, ESP32CAM waits for MQTT message before firing
}

esp_err_t handleMQTT(esp_mqtt_event_handle_t event) {
  mqttClient.onEventCallback(event);
  return ESP_OK;
}

void onConnectionEstablishedCallback(esp_mqtt_client_handle_t client) {
  if (mqttClient.isMyTurn(client)) {
    mqttClient.subscribe(gantryCameraTopic, [](const String &message) {
      Serial.println(gantryCameraTopic + ": " + message);
      if (strcmp(message.c_str(), "1") == 0) {
        delay(3000);
        captureAndSendImage();
        // captureAndSendImageWithId();
      }
    });
  }
}

// This function is written by Sean.
void sendImage() {
  // Dummy to remove the camera lag
  // camera_fb_t * fb_dummy = esp_camera_fb_get();
  // esp_camera_fb_return(fb_dummy);

  const int flashLED = 4; // Adjust the pin number if necessary

  // Configure the flash LED pin as an output
  pinMode(flashLED, OUTPUT);
  // digitalWrite(flashLED, HIGH); // Turn on the flash LED
  analogWrite(flashLED, intFlashPower);

  Serial.println("FLASH ON");
  camera_fb_t * fb = esp_camera_fb_get();
  Serial.println("GET FRAME BUFFER");
  if (!fb) {
    esp_camera_fb_return(fb);
    Serial.println("Camera capture failed");
    // digitalWrite(flashLED, LOW);
    analogWrite(flashLED, 0);
    return;
  }

  // Post image to Flask server
  HTTPClient http;
  http.begin("http://192.168.43.156:3237/backend/carplate-recognition/entrance"); // PUT YOUR SERVER ADDRESS HERE
  http.addHeader("Content-Type", "image/jpeg");

  int httpResponseCode = http.POST(fb->buf, fb->len);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode); 
  }

  http.end();
  esp_camera_fb_return(fb);
  // digitalWrite(flashLED, LOW);
  analogWrite(flashLED, 0);
  Serial.println("Fb returned");
}


// Function modofied from from Tang Ze Hou
// Dependent on WiFiClient
void captureAndSendImage() {
  
  const int flashLED = 4; // Adjust the pin number if necessary

  // Configure the flash LED pin as an output
  pinMode(flashLED, OUTPUT);
  // digitalWrite(flashLED, HIGH); // Turn on the flash LED
  analogWrite(flashLED, intFlashPower);

  Serial.println("Attempting to get Frame Buffer");
  camera_fb_t * fb = esp_camera_fb_get();

 //Giving it 2s to capture
  delay(2000);

  if (!fb) {
    Serial.println("Camera capture failed");
    analogWrite(flashLED, 0);
    return;
  }
  analogWrite(flashLED, 0);
  Serial.println("Image in Frame Buffer");
  WiFiClient client;
  if (!client.connect(backendHost, backendPort)) {
    Serial.println("Connection to server failed");
    return;
  }
  Serial.println("Connected to server");
  String boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
  String head = "--" + boundary + "\r\nContent-Disposition: form-data; name=\"file\"; filename=\"image.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
  String tail = "\r\n--" + boundary + "--\r\n";
  client.println("POST " + strApiRoute + " HTTP/1.1");
  client.println("Host: " + String(backendHost) + ":" + String(backendPort));
  client.println("Content-Length: " + String(fb->len + head.length() + tail.length()));
  client.println("Content-Type: multipart/form-data; boundary=" + boundary);
  client.println();
  client.print(head);
  uint8_t *fbBuf = fb->buf;
  size_t fbLen = fb->len;
  for (size_t n = 0; n < fbLen; n = n + 1024) {
    if (n + 1024 < fbLen) {
      client.write(fbBuf, 1024);
      fbBuf += 1024;
    } else if (fbLen % 1024 > 0) {
      size_t remainder = fbLen % 1024;
      client.write(fbBuf, remainder);
    }
  }
  client.print(tail);
  int timeout = 10000; // 10 seconds timeout for reading the data
  long int startTime = millis();
  while (client.connected() && millis() - startTime < timeout) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Headers received");
      break;
    }
  }

  String line = client.readStringUntil('\n');
  Serial.println(line);
  
  // Close the connection
  client.stop();
  Serial.println("Connection closed");
  esp_camera_fb_return(fb);
  Serial.println("Frame buffer returned");
}

void captureAndSendImageWithId() {
  
  const int flashLED = 4; // Adjust the pin number if necessary

  // Configure the flash LED pin as an output
  pinMode(flashLED, OUTPUT);
  // digitalWrite(flashLED, HIGH); // Turn on the flash LED
  analogWrite(flashLED, intFlashPower);

  Serial.println("Attempting to get Frame Buffer");
  camera_fb_t * fb = esp_camera_fb_get();

 //Giving it 2s to capture
  delay(3000);

  if (!fb) {
    Serial.println("Camera capture failed");
    analogWrite(flashLED, 0);
    return;
  }

  Serial.println("HERE!");
  analogWrite(flashLED, 0);
  Serial.println("Image in Frame Buffer");
  WiFiClient client;
  if (!client.connect(backendHost, backendPort)) {
    Serial.println("Connection to server failed");
    return;
  }
  Serial.println("Connected to server");
  String boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
  String head = "--" + boundary + "\r\nContent-Disposition: form-data; name=\"file\"; filename=\"image.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
  String tail = "\r\n--" + boundary + "--\r\n";
  client.println("POST " + strApiRoute + " HTTP/1.1");
  client.println("Host: " + String(backendHost) + ":" + String(backendPort));
  client.println("Content-Length: " + String(fb->len + head.length() + tail.length()));
  client.println("ID:" + String(strLotId));
  client.println("Content-Type: multipart/form-data; boundary=" + boundary);
  client.println();
  client.print(head);
  uint8_t *fbBuf = fb->buf;
  size_t fbLen = fb->len;
  for (size_t n = 0; n < fbLen; n = n + 1024) {
    if (n + 1024 < fbLen) {
      client.write(fbBuf, 1024);
      fbBuf += 1024;
    } else if (fbLen % 1024 > 0) {
      size_t remainder = fbLen % 1024;
      client.write(fbBuf, remainder);
    }
  }
  client.print(tail);
  int timeout = 10000; // 10 seconds timeout for reading the data
  long int startTime = millis();
  while (client.connected() && millis() - startTime < timeout) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Headers received");
      break;
    }
  }

  String line = client.readStringUntil('\n');
  Serial.println(line);
  
  // Close the connection
  client.stop();
  Serial.println("Connection closed");
  esp_camera_fb_return(fb);
  Serial.println("Frame buffer returned");
}