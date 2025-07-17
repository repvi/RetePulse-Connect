#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "mqtt_client.h"
#include "esp_http_client.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MQTT_ENABLED   true
#define MQTT_DISABLED  false

typedef struct MqttMaintainer MqttMaintainer;
typedef MqttMaintainer *MqttMaintainerHandler;

typedef struct {
    char *last_updated;
    char *sensor_type;
    char *model;
} mqtt_device_info_t;

typedef enum {
    MQTT_DEVICE_STATUS_HEAP_ERROR = -3,
    MQTT_DEVICE_STATUS_ERROR = -2,
    MQTT_DEVICE_STATUS_DISCONNECTED = -1,
    MQTT_DEVICE_STATUS_CONNECTED,
    MQTT_DEVICE_STATUS_SLEEPING,
    MQTT_DEVICE_STATUS_AWAITING_SLEEP
} mqtt_device_status_t;

/**
 * @brief Send single key-value pair as JSON to MQTT topic
 * @param topic MQTT topic
 * @param key   JSON key
 * @param data  JSON value
 * @return >0 on success, -1 on JSON fail, -2 if buffer too small
 */
int send_to_mqtt_service_single(MqttMaintainerHandler handler, char *const topic, char const *const key, const char *const data);

/**
 * @brief Send multiple key-value pairs as JSON to MQTT topic
 * @param topic MQTT topic
 * @param key   Array of JSON keys
 * @param data  Array of JSON values
 * @param len   Number of key-value pairs
 * @return >0 on success, -1 on JSON fail, -2 if buffer too small
 */
int send_to_mqtt_service_multiple(MqttMaintainerHandler handler, char *const topic, const char**key, const char**data, int len);

/**
 * @brief Send device status update to MQTT broker
 * 
 * @description
 * Publishes device operational status as JSON to the device status topic
 * for health monitoring and system state reporting.
 * 
 * @param handler Valid MqttMaintainerHandler from init_mqtt()
 * @param status Device status enumeration:
 *               - MQTT_DEVICE_STATUS_CONNECTED: Device operational
 *               - MQTT_DEVICE_STATUS_DISCONNECTED: Device offline
 *               - MQTT_DEVICE_STATUS_SLEEPING: Power-save mode
 *               - MQTT_DEVICE_STATUS_AWAITING_SLEEP: Preparing for sleep
 *               - MQTT_DEVICE_STATUS_HEAP_ERROR: Memory failure
 *               - MQTT_DEVICE_STATUS_ERROR: General error condition
 * 
 * @return int Operation result:
 *         - Positive: MQTT message ID for tracking
 *         - -1: Topic buffer overflow or JSON failure
 *         - -2: MQTT publish failed
 *         - -3: Invalid parameters
 * 
 * @json_output {"status": "connected"|"disconnected"|"sleeping"|"awaiting_sleep"|"heap_error"|"error"|"unknown"}
 * @topic_format "device/{device_id}/status"
 * 
 * @preconditions Valid handler, active MQTT connection
 * @thread_safety Thread-safe with valid handler
 * @memory_usage ~64 bytes stack for topic string
 * 
 * @usage_frequency On startup/shutdown, power transitions, errors, periodic heartbeat
 */
int send_mqtt_device_status(MqttMaintainerHandler handler, mqtt_device_status_t status);

/**
 * @brief MQTT event handler for ESP-IDF MQTT client.
 *
 * Handles various MQTT events such as connection, data reception, and others.
 * - On connection, subscribes to "home/sensor" and "home/led" topics.
 * - On data reception, prints the topic and data, and performs actions based on topic/data:
 *     - For "home/led": (example) would toggle an LED based on "on"/"off" message.
 *     - For "home/ota": (example) would trigger OTA update if "update" message is received.
 *
 * @param handler_args  User-defined handler arguments (unused).
 * @param base          Event base (unused).
 * @param event_id      Event ID (MQTT event type).
 * @param event_data    Pointer to event-specific data (esp_mqtt_event_handle_t).
 */
void mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data);

/**
 * @brief Initialize MQTT client service with specified parameters
 *
 * @param url          MQTT broker URL
 * @param buffer_size  Size for receive buffer (minimum 1024 bytes)
 * @param out_size     Size for output buffer (minimum 512 bytes)
 *
 * @return ESP_OK if initialization successful
 *         ESP_FAIL if semaphore creation fails
 *         Other ESP errors from MQTT client operations
 *
 * Function flow:
 * 1. Creates and initializes mutex for thread safety
 * 2. Adjusts buffer sizes to minimums if needed
 * 3. Configures and starts MQTT client
 */
MqttMaintainerHandler init_mqtt(esp_mqtt_client_config_t *mqtt_cfg, const mqtt_device_info_t *device_info);

/**
 * @brief Deinitialize MQTT service and clean up resources
 *
 * Frees allocated memory, destroys mutex, and stops MQTT client.
 * Should be called when MQTT service is no longer needed.
 */
void mqtt_service_deinit(MqttMaintainerHandler handler);

#ifdef __cplusplus
}
#endif