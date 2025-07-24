#pragma once

#include "mqtt_operation.h"
#include "ota_operation.h"
#include "wifi_operation.h"
#include "parsing.h"
#include "hashmap.h"
#include "stdatomic.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_log.h"

/**
 * @brief Data package for MQTT event handling.
 */
struct mqtt_data_package_t {
    esp_mqtt_event_handle_t event; ///< MQTT event handle
    cJSON *json; ///< JSON data parsed from event
};

/**
 * @brief Function pointer type for handling MQTT event data.
 */
typedef void (*mqtt_event_data_action_t)(mqtt_data_package_t *package);

namespace MicroUSC {

    /**
     * @brief Maintains MQTT client connection, subscriptions, and message publishing.
     *
     * Usage:
     * - Call start() to initialize and connect.
     * - Use addMqttClientSubscribe() to subscribe to topics.
     * - Use sendToMqttServiceSingle()/sendToMqttServiceMultiple() to publish data.
     */
    class MqttMaintainer {
        public:
        static const int STRING_SIZE = 32; ///< Size for string buffers

        /**
         * @brief Start the MQTT client with the given configuration.
         * @param config MQTT client configuration
         * @return ESP_OK on success
         */
        esp_err_t start(const esp_mqtt_client_config_t &config);

        /**
         * @brief Get device name.
         */
        const char *getDeviceName() const;
        /**
         * @brief Get last updated timestamp.
         */
        const char *getLastUpdated() const;
        /**
         * @brief Get sensor type.
         */
        const char *getSensorType() const;

        /**
         * @brief Subscribe to an MQTT topic with a custom action handler.
         * @param topic Topic string
         * @param qos Quality of Service level
         * @param action Handler function for received data
         * @return true on success
         */
        bool addMqttClientSubscribe(const char *topic, int qos, mqtt_event_data_action_t action);

        /**
         * @brief Send a single key-value pair as JSON to an MQTT topic.
         */
        int sendToMqttServiceSingle(char *const topic, char const *const key, const char *const data);

        /**
         * @brief Send multiple key-value pairs as JSON to an MQTT topic.
         */
        int sendToMqttServiceMultiple(char *const topic, const char**key, const char**data, int len);

        /**
         * @brief Convert function pointer to void* for storage.
         */
        static void *convertFuncToIntptr(mqtt_event_data_action_t action);

        /**
         * @brief Convert void* back to function pointer.
         */
        static mqtt_event_data_action_t convertIntptrToFunc(void *ptr);

        private:
    
        esp_err_t setupMqttService(int &buffer_size, int &out_size);

        int sendConnectionInfo();
        void mqttDataRecievedHandler(mqtt_data_package_t *package);
        void mqttConnectHandler();
        void mqttReconnectHandler();
        void mqttDataHandler(esp_mqtt_event_handle_t event);
        void mqttEventHandler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data);
        
        int check_device_name(const char *new_name);

        static void mqttEventHandlerHelper(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data);

        esp_mqtt_client_config_t config;
        esp_mqtt_client_handle_t client;

        HashMap mqtt_device_map;
    
        StaticSemaphore_t semaphore_buffer;
        SemaphoreHandle_t mutex;

        char name[STRING_SIZE];
        char last_updated[STRING_SIZE];
        char sensor_type[STRING_SIZE];
    };
} // namespace MicroUSC