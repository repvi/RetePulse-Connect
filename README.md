| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- | -------- |

# ESP-IDF Sample Project

This repository provides a minimal, buildable ESP-IDF project template for ESP32-based devices. It demonstrates best practices for project structure, component organization, and integration of WiFi and MQTT functionality.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Project Structure](#project-structure)
- [Components](#components)
- [Usage](#usage)
- [Getting Started](#getting-started)
- [License](#license)

## Overview

This project is intended as a starting point for new ESP-IDF applications. It includes example implementations for WiFi connectivity, MQTT communication, and a simple hashmap utility for key-value storage.  
You can use the `idf.py create-project` command to copy this template and set your project name.  
For more details, refer to the [ESP-IDF build system documentation](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project).

## Features

- WiFi connection management
- MQTT client setup, publish, subscribe, and event handling
- Simple hashmap implementation for fast key-value lookups
- Modular and extensible component structure

## Project Structure

```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   └── main.c
├── components
│   └── wireless
│       ├── hashmap.c
│       ├── include
│       │   ├── hashmap.h
│       │   ├── mqtt_handler.hpp
│       │   ├── mqtt_operation.h
│       │   ├── wifi_handler.hpp
│       ├── mqtt_operation.cpp
├── README.md
```

## Components

- **wireless/hashmap.c / hashmap.h**  
  Implements a lightweight hashmap for key-value storage, optimized for embedded systems.

- **wireless/include/mqtt_handler.hpp / mqtt_operation.h / mqtt_operation.cpp**  
  Provides MQTT client management, event handling, and message publishing/subscribing.  
  - `MqttMaintainer` class: Main interface for MQTT operations.
  - Functions for sending single/multiple key-value pairs as JSON to MQTT topics.
  - Event handler for MQTT events.

- **wireless/include/wifi_handler.hpp**  
  Manages WiFi connection and event handling.

- **esp_system.h**  
  ESP-IDF system functions for reset, heap management, and shutdown handlers.

- **esp-mqtt/include/mqtt_client.h**  
  ESP-IDF MQTT client API for connecting, publishing, subscribing, and event handling.

## Usage

- Use `init_mqtt()` to initialize the MQTT client with configuration and device info.
- Use `send_to_mqtt_service_single()` and `send_to_mqtt_service_multiple()` to publish data.
- Use `add_esp_mqtt_client_subscribe()` to subscribe to topics with custom handlers.
- WiFi setup is managed via the `WifiMaintainer` class.

## Getting Started

1. Clone this repository.
2. Configure your project and WiFi/MQTT settings in the relevant source/header files.
3. Build the project using CMake and ESP-IDF tools:
   ```
   idf.py build
   ```
4. Flash the firmware to your ESP32 device:
   ```
   idf.py -p <PORT> flash
   ```
5. Monitor the device output:
   ```
   idf.py -p <PORT> monitor
   ```

## License

This project is licensed under the Apache License 2.0. See the [LICENSE](LICENSE) file for details.

---
# Other Software
What to connect the esp32s to the main software? Please checkout [RetePulse](https://github.com/repvi/RetePulse)