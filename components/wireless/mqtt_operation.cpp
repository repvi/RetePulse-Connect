#include "mqtt_handler.hpp"
#include "mqtt_operation.h"
#include "ota_operation.h"
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
    MicroUSC::MqttMaintainer *maintainer = reinterpret_cast<MicroUSC::MqttMaintainer *>(handler);
    return maintainer->sendToMqttServiceSingle(topic, key, data);
}

extern "C" int send_to_mqtt_service_multiple(MqttMaintainerHandler handler, char *const topic, const char**key, const char**data, int len)
{
    MicroUSC::MqttMaintainer *maintainer = reinterpret_cast<MicroUSC::MqttMaintainer *>(handler);
    return maintainer->sendToMqttServiceMultiple(topic, key, data, len);
}

extern "C" bool add_esp_mqtt_client_subscribe(
    MqttMaintainerHandler handler,
    const char *topic, 
    int qos,
    mqtt_event_data_action_t action
) {
    MicroUSC::MqttMaintainer *maintainer = reinterpret_cast<MicroUSC::MqttMaintainer *>(handler);
    return maintainer->addMqttClientSubscribe(topic, qos, action);
}

extern "C" MqttMaintainerHandler init_mqtt(esp_mqtt_client_config_t *mqtt_cfg, const mqtt_device_info_t *device_info)
{
    if (check_connection() != ESP_OK) {
        ESP_LOGE(TAG, "No WiFi connection available");
        return NULL; // Return failure if no WiFi connection
    }

    void *mem = heap_caps_malloc(sizeof(MicroUSC::MqttMaintainer), MALLOC_CAP_8BIT | MALLOC_CAP_DMA);
    if (mem == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for MQTT maintainer handler");
        return NULL; // Return memory allocation error
    }

    MicroUSC::MqttMaintainer *handler = reinterpret_cast<MicroUSC::MqttMaintainer *>(mem); /* Create a new MQTT maintainer handler */

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
    MicroUSC::MqttMaintainer *maintainer = reinterpret_cast<MicroUSC::MqttMaintainer *>(handler);
    if (maintainer) {
        // maintainer->stop();
        heap_caps_free(maintainer);
    }
}