# Paktani IoT ESP32 Gateway Firmware

This project implements an ESP-IDF firmware for an ESP32-based IoT gateway in the PAKTANI IOT ecosystem. It is designed as a Modbus master that connects to a WiFi access point, polls Modbus slave devices for sensor data, timestamps the data with an RTC (DS3231), and stores the data locally in a FIFO queue as backup. Future integration of MQTT data forwarding is planned.

---

## Table of Contents

- [Paktani IoT ESP32 Gateway Firmware](#paktani-iot-esp32-gateway-firmware)
  - [Table of Contents](#table-of-contents)
  - [Overview](#overview)
  - [Features](#features)
  - [Hardware Requirements](#hardware-requirements)
  - [Software and Libraries](#software-and-libraries)
  - [Program Flow and Architecture](#program-flow-and-architecture)
    - [1. WiFi Task](#1-wifi-task)
    - [2. Modbus Task](#2-modbus-task)
    - [3. LED Task](#3-led-task)
    - [Main Loop](#main-loop)
  - [File Structure](#file-structure)
  - [Setup and Build Instructions](#setup-and-build-instructions)

---

## Overview

The firmware acts as a gateway in a Modbus network:
- **WiFi Connection:** Uses a custom `Wifi` library to connect and maintain a connection to a WiFi access point. It monitors the connection status, and if the connection drops, a global flag is set that can trigger a change in program mode.
- **Modbus Polling:** Implements a Modbus master using the `ModbusRTU` class to sequentially poll multiple Modbus slave devices. The slave responses are read from holding registers that include sensor data such as device status, humidity, and temperature.
- **Timestamping with RTC:** Uses the DS3231 RTC (via the `DS3231` class) to obtain a current timestamp. The timestamp is paired with each set of sensor data.
- **Local Storage:** Collected sensor data along with the timestamp is stored in a FIFO queue. This local backup is designed to preserve sensor readings until they can be forwarded to an MQTT server.
- **Visual Feedback:** A simple LED (controlled via a `Gpio` class) toggles at a regular interval as a status indicator.

---

## Features

- **WiFi Connectivity:**  
  Connects to an access point using configurable credentials. Continuously monitors the connection status.
  
- **Modbus Communication:**  
  Supports communication with multiple Modbus slave devices through a UART interface.
  
- **Real-Time Clock Integration:**  
  Provides precise timestamps for each sensor reading using a DS3231 RTC over I2C.
  
- **Local Data Backup:**  
  Stores sensor data in a FIFO queue ensuring data is backed up locally before being sent to the cloud.
  
- **LED Indicator:**  
  Uses an onboard LED for visual feedback of system status.
  
- **Extensible Architecture:**  
  Designed to allow future integration with MQTT for cloud connectivity and data forwarding.

---

## Hardware Requirements

- **ESP32/ESP32S3 Development Board**
- **DS3231 RTC Module** (connected via I2C)
- **Modbus Slave Devices** (using RS-485 with UART interface)
- **LED** (for status indication, e.g., on GPIO2)
- **Proper wiring and power supply for connected devices**

---

## Software and Libraries

The firmware leverages several custom and ESP-IDF libraries:

- **Wifi.h:**  
  Handles WiFi initialization, connection, and event management.

- **DS3231.h:**  
  Provides functions to initialize the DS3231 RTC, set/get time, and retrieve temperature readings.

- **I2CMaster.h:**  
  Wraps ESP-IDF I2C functionality for easier communication with I2C devices.

- **Modbus.h / ModbusRTU:**  
  Implements a Modbus RTU master for polling sensor data from slave devices.

- **Gpio.h:**  
  Provides a simple abstraction to control GPIO pins, e.g., toggling an LED.

- **FreeRTOS:**  
  Used for task creation and scheduling for concurrent operations (WiFi, Modbus polling, LED blinking).

---

## Program Flow and Architecture

The firmware is structured around three primary FreeRTOS tasks, each handling a specific subsystem:

### 1. WiFi Task
- **Initialization:**  
  Uses the `Wifi` library to set SSID and password.
- **Connection Management:**  
  Calls `init()` and `connect()` to join the access point.  
  Monitors connection status and sets a global flag (`wifiConnected`) if disconnected.
- **Status Notification:**  
  Logs status changes and can trigger a different program mode when the connection drops.

### 2. Modbus Task
- **RTC Initialization:**  
  Instantiates an `I2CMaster` and initializes the DS3231 RTC.
- **Modbus Polling:**  
  Creates instances of `ModbusRTU` for each slave device.  
  Sequentially polls each slave by reading holding registers (for device status, humidity, and temperature).
- **Timestamping:**  
  Retrieves the current time from the RTC for each sensor read.
- **Local Storage:**  
  Combines sensor data and the RTC timestamp into a `SensorRecord` structure and stores it in a FIFO queue.
- **Data Conversion:**  
  Converts raw register values to floating-point numbers using a helper function.

### 3. LED Task
- **Visual Indicator:**  
  Toggles an LED periodically to provide a simple indication that the firmware is running.

### Main Loop
- **Data Processing:**  
  Continuously monitors the FIFO queue for new sensor data.  
  Logs the data and serves as the integration point for future MQTT forwarding.

---

## File Structure

```
Paktani_IOT_ESP32_Gateway/
├── components/
│   ├── Wifi/            // Contains Wifi.h and Wifi.cpp
│   ├── DS3231/          // Contains DS3231.h and DS3231.cpp
│   ├── I2CMaster/       // Contains I2CMaster.h and I2CMaster.cpp
│   ├── Modbus/          // Contains Modbus.h, Modbus.cpp, and ModbusRTU implementation
│   └── Gpio/            // Contains Gpio.h and Gpio.cpp
├── main/
│   └── main.cpp         // Contains the application entry point and task implementations
├── CMakeLists.txt       // Build configuration for ESP-IDF
└── README.md            // This documentation file
```

---

## Setup and Build Instructions

1. **Install ESP-IDF:**  
   Follow the official [ESP-IDF installation guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html).

2. **Clone the Repository:**  
   ```bash
   git clone https://your.repo.url/Paktani_IOT_ESP32_Gateway.git
   cd Paktani_IOT_ESP32_Gateway
   ```

3. **Configure the Project:**  
   Set your WiFi credentials and adjust pin assignments in `main.cpp` or via menuconfig.
   ```bash
   idf.py menuconfig
   ```

4. **Build the Firmware:**  
   ```bash
   idf.py build
   ```

5. **Flash to the ESP32:**  
   Connect your ESP32 board and run:
   ```bash
   idf.py flash monitor
   ```

---
