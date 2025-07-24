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

struct mqtt_data_package_t {
    esp_mqtt_event_handle_t event;
    cJSON *json; // JSON data parsed from event
};

typedef void (*mqtt_event_data_action_t)(mqtt_data_package_t *package);

namespace MicroUSC {
    class MqttMaintainer {
        public:

        static const int STRING_SIZE = 32; // Size for string buffers
        
        esp_err_t start(const esp_mqtt_client_config_t &config);
    
        const char *getDeviceName() const;
        const char *getLastUpdated() const;
        const char *getSensorType() const;

        bool addMqttClientSubscribe(const char *topic, int qos, mqtt_event_data_action_t action);

        int sendToMqttServiceSingle(char *const topic, char const *const key, const char *const data);
        int sendToMqttServiceMultiple(char *const topic, const char**key, const char**data, int len);

        static void *convertFuncToIntptr(mqtt_event_data_action_t action);
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
}