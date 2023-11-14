# Smart Carpark Arduino Codebase 🚗💡

Welcome to the GitHub repository for the Smart Carpark system's Arduino codes! This code repository is the driving force behind our innovative Smart Carpark, steering the functionality of vehicle detection, plate recognition, and more.

This is done as part of the Project for CS3237 - Introduction to Internet of Things, AY23/24 Semester 1.

Done by Group 5!

## Repository Structure 🗂️

Here's a guide to what you'll find in this repository:

- `Camera/`: Contains all arduino sketch for ESP32 Camera necessary for operating the ESP32 Camera to capture image for car plate recognition and human detection functionalities.

- `Lot/`: Includes arduino sketch for ESP32 to manage and monitor parking lot statuses.

- `Entrance/`: Dedicated to the arduino sketch for ESP32 controlling the entry gate sensor logic and mechanisms.

- `Exit/`: Houses the arduino sketch for ESP32 that manages the logic and mechanisms for the carpark's exit gate.

- `Testing/`: This is where you find the code we use to test the MQTT commands, as well as the code to subscribe to all topics, we use this for monitoring

## Setup & Installation ⚙️

To set up your environment for contributing to this project, please follow these steps:

1. Ensure you have the Arduino IDE installed on your computer.
2. Ensure you have MQTT server set up.
3. Install the ESP32 Board Manager in your Arduino IDE:
   - Go to `File > Preferences`.
   - In the "Additional Boards Manager URLs" field, enter `https://dl.espressif.com/dl/package_esp32_index.json` and click "OK".
   - Go to `Tools > Board > Boards Manager...`.
   - In the search bar, type "esp32" and install the board that appears.
4. Install all necessary libraries as listed in the Dependencies section.
5. Connect your ESP32 / ESP32 Camera module and any other relevant hardware to your computer.

## Usage 🛠️

To use the code:

1. Navigate to the desired project folder.
2. Open the Arduino sketch (.ino file) with the Arduino IDE.
3. Compile and upload the sketch to your Arduino module following the IDE's upload protocol.

## Dependencies 📚

This project requires the following libraries:

- For `Camera/`:
  - WiFi@2.0.0
  - ESP32MQTTClient@0.1.0
  - HTTPClient@2.0.0
  - WiFiClientSecure@2.0.0

- For `Lot/`:
  - WiFi@2.0.0
  - ESP32MQTTClient@0.1.0

- For `Entrance/` and `Exit/`:
  - LiquidCrystal_I2C@1.1.4
  - Wire@2.0.0
  - ESP8266 and ESP32 OLED driver for SSD1306 displays@4.4.0
  - ESP32Servo@1.1.0
  - ESP32MQTTClient@0.1.0
  - WiFi@2.0.0

Please ensure that you have these libraries installed before running the project.

You can install these libraries through your Arduino IDE's Library Manager. Go to `Sketch > Include Library > Manage Libraries...` and search for the above libraries. Click on the library and then click on `Install`.

For more detailed instructions on how to install an Arduino library, you can refer to the [official Arduino guide](https://www.arduino.cc/en/guide/libraries).


---
We're excited to see how you'll help us drive the future of smart parking! 🌟

Happy coding! 🚀👩‍💻👨‍💻\
CS3237 Group G05