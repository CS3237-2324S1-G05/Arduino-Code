# Smart Carpark Arduino Codebase 🚗💡

Welcome to the GitHub repository for the Smart Carpark system's Arduino codes! This code repository is the driving force behind our innovative Smart Carpark, steering the functionality of vehicle detection, plate recognition, and more.

This is done as part of the Project for CS3237 - Introduction to Internet of Things, AY23/24 Semester 1.

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
2. Install all necessary libraries and ESP32 Board manager
3. Connect your ESP32 / ESP32 Camera module and any other relevant hardware to your computer.

## Usage 🛠️

To use the code:

1. Navigate to the desired project folder.
2. Open the Arduino sketch (.ino file) with the Arduino IDE.
3. Compile and upload the sketch to your Arduino module following the IDE's upload protocol.

---
We're excited to see how you'll help us drive the future of smart parking! 🌟

Happy coding! 🚀👩‍💻👨‍💻\
CS3237 Group G05