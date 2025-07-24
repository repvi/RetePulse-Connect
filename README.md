
| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- | -------- |

# RetePulse: ESP32 MQTT Wireless Framework

## Overview

**RetePulse** is a modular wireless communication framework for ESP32 devices, focused on robust MQTT messaging, device management, and OTA updates. It is designed for IoT applications requiring reliable, secure, and efficient device-to-cloud communication.

### Features

- High-level MQTT client with automatic connection management
- Thread-safe topic subscription and message publishing
- JSON-based message handling with memory pooling
- Device status reporting and health monitoring
- Over-the-air (OTA) firmware update support
- Configurable for multiple ESP32 targets

---

## Project Structure

```
├── components
│   └── wireless
│       ├── include
│       │   ├── mqtt_handler.hpp      # Main MQTT client class and types
│       │   ├── mqtt_operation.h      # C interface for MQTT operations
│       │   ├── wifi_operation.h      # WiFi management
│       │   ├── parsing.h             # JSON parsing utilities
│       │   ├── hashmap.h             # Lightweight hashmap for subscriptions
│       ├── mqtt_handler.cpp          # MQTT client implementation
│       ├── mqtt_operation.cpp        # C wrappers for MQTT operations
│       ├── hashmap.c                 # Hashmap implementation
│       └── ...                       # Other wireless components
├── main
│   ├── main.c                        # Application entry point
│   └── CMakeLists.txt
├── CMakeLists.txt
└── README.md                         # Project documentation
```

---

## Getting Started

### Prerequisites

- ESP-IDF v5.3.2 or newer
- Supported ESP32 hardware
- Python 3.x (for build tools)
- CMake (used by ESP-IDF build system)

### Build Instructions

1. **Set up ESP-IDF:**  
   Follow [Espressif's setup guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html).

2. **Configure the project:**  
   ```sh
   idf.py menuconfig
   ```

3. **Build the project:**  
   ```sh
   idf.py build
   ```

4. **Flash to device:**  
   ```sh
   idf.py -p <PORT> flash
   ```

5. **Monitor output:**  
   ```sh
   idf.py -p <PORT> monitor
   ```

---

## Key Components Documentation

### MQTT Handler (`mqtt_handler.hpp` / `.cpp`)

- **Class:** `RetePulse::MqttMaintainer`
  - Manages MQTT connection, subscriptions, and message publishing.
  - Thread-safe via FreeRTOS binary semaphore.
  - Supports device info, OTA, and control topics.
- **Struct:** `mqtt_data_package_t`
  - Used for passing event data to callbacks.
- **Functionality:**
  - `start()`, `stop()`: Lifecycle management.
  - `addMqttClientSubscribe()`: Subscribe to topics with custom handlers.
  - `sendToMqttServiceSingle()`, `sendToMqttServiceMultiple()`: Publish JSON messages.
  - OTA and control command handling.

### MQTT Operation (`mqtt_operation.h` / `.cpp`)

- **C wrappers** for MQTT operations, allowing integration with C codebases.
- Functions:
  - `send_to_mqtt_service_single()`
  - `send_to_mqtt_service_multiple()`
  - `send_mqtt_device_status()`
- Designed for interoperability and easy extension.

### Hashmap (`hashmap.h` / `.c`)

- Lightweight, fixed-size hashmap for topic subscription management.
- Uses power-of-2 sizing for fast lookup.
- Functions:
  - `hashmap_create()`, `hashmap_init()`
  - `hashmap_put()`, `hashmap_get()`, `hashmap_remove()`

### Parsing (`parsing.h`)

- Utilities for extracting and validating JSON data from MQTT messages.
- Uses cJSON for efficient parsing.

---

## Example Usage

```cpp
// Subscribe to a topic
mqttMaintainer.addMqttClientSubscribe("device/control", 1, control_callback);

// Publish device status
send_mqtt_device_status(mqttMaintainerHandler, MQTT_DEVICE_STATUS_CONNECTED);

// Handle OTA update
void ota_callback(mqtt_data_package_t *package) {
    // Start OTA update using esp_https_ota
}
```

---

## OTA Updates

- Uses `esp_https_ota.h` for secure firmware updates.
- OTA topic is configurable; device listens for update commands and triggers OTA process.

---

## Extending the Framework

- Add new MQTT topics and handlers by extending `MqttMaintainer`.
- Integrate additional wireless protocols by adding new components under `components/wireless`.
- Use the provided hashmap for efficient resource management.

---

## Troubleshooting

- **Failed to subscribe to topic:**  
  Ensure topic strings are valid and buffer sizes are sufficient.
- **MQTT_EVENT_PUBLISHED not received:**  
  Use QoS 1 or 2 for delivery confirmation.
- **OTA update issues:**  
  Check OTA server URL and certificate configuration.

---

## License

MIT License. See [LICENSE](LICENSE) for details.

---

## Authors & Credits

Developed by the RetePulse Team.  
Based on ESP-IDF and cJSON libraries.

---

## References

- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [MQTT Protocol Specification](http://mqtt.org/documentation)
- [cJSON Library](https://github.com/DaveGamble/cJSON)

---

*For further documentation, see comments in each source/header file and the ESP-IDF API guides.*
