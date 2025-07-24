#include "mqtt_operation.h"
#include "mqtt_handler.hpp"
#include "wifi_operation.h"
#include "parsing.h"
#include "hashmap.h"
#include "esp_heap_caps.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_log.h"

#define TAG "[MQTT SERVICE]"

extern "C" int send_to_mqtt_service_single(MqttMaintainerHandler handler, char *const topic, char const *const key, const char *const data)
{
    RetePulse::MqttMaintainer *maintainer = reinterpret_cast<RetePulse::MqttMaintainer *>(handler);
    return maintainer->sendToMqttServiceSingle(topic, key, data);
}

extern "C" int send_to_mqtt_service_multiple(MqttMaintainerHandler handler, char *const topic, const char**key, const char**data, int len)
{
    RetePulse::MqttMaintainer *maintainer = reinterpret_cast<RetePulse::MqttMaintainer *>(handler);
    return maintainer->sendToMqttServiceMultiple(topic, key, data, len);
}

extern "C" int send_mqtt_device_status(MqttMaintainerHandler handler, mqtt_device_status_t status)
{
    char topic[48];
    RetePulse::MqttMaintainer *maintainer = reinterpret_cast<RetePulse::MqttMaintainer *>(handler);
    int written = snprintf(topic, sizeof(topic), "%s%s", RetePulse::MqttMaintainer::statusTopic, maintainer->getName());
    if (written > sizeof(topic)) {
        ESP_LOGE(TAG, "Topic buffer too small for status %d", status);
        return -1; // Indicate failure
    }

    constexpr const char *key = "status";
    const char *data = nullptr;

    switch (status) {
        case MQTT_DEVICE_STATUS_CONNECTED:
            data = "connected";
            break;
        case MQTT_DEVICE_STATUS_DISCONNECTED:
            data = "disconnected";
            break;
        case MQTT_DEVICE_STATUS_SLEEPING:
            data = "sleeping";
            break;
        case MQTT_DEVICE_STATUS_AWAITING_SLEEP:
            data = "awaiting_sleep";
            break;
        case MQTT_DEVICE_STATUS_HEAP_ERROR:
            data = "heap_error";
            break;
        case MQTT_DEVICE_STATUS_ERROR:
            data = "error";
            break;
        default:
            data = "unknown";
            break;
    }

    return send_to_mqtt_service_single(handler, topic, key, data);
}

extern "C" bool add_esp_mqtt_client_subscribe(
    MqttMaintainerHandler handler,
    const char *topic, 
    int qos,
    mqtt_event_data_action_t action
) {
    RetePulse::MqttMaintainer *maintainer = reinterpret_cast<RetePulse::MqttMaintainer *>(handler);
    return maintainer->addMqttClientSubscribe(topic, qos, action);
}

extern "C" MqttMaintainerHandler init_mqtt(esp_mqtt_client_config_t *mqtt_cfg, const mqtt_device_info_t *device_info)
{
    if (check_connection() != ESP_OK) {
        ESP_LOGE(TAG, "No WiFi connection available");
        return NULL; // Return failure if no WiFi connection
    }

    void *mem = heap_caps_malloc(sizeof(RetePulse::MqttMaintainer), MALLOC_CAP_8BIT | MALLOC_CAP_DMA);
    if (mem == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for MQTT maintainer handler");
        return NULL; // Return memory allocation error
    }

    RetePulse::MqttMaintainer *handler = reinterpret_cast<RetePulse::MqttMaintainer *>(mem); /* Create a new MQTT maintainer handler */

    esp_err_t err = handler->start(*mqtt_cfg);
    if (err == ESP_OK) {
        return reinterpret_cast<MqttMaintainerHandler>(handler);
    }
    else {
        heap_caps_free(mem); // Free allocated memory on failure
        return NULL;
    }
}

extern "C" void mqtt_service_deinit(MqttMaintainerHandler handler) 
{
    RetePulse::MqttMaintainer *maintainer = reinterpret_cast<RetePulse::MqttMaintainer *>(handler);
    if (maintainer) {
        maintainer->stop();
        heap_caps_free(maintainer);
    }
}