#include "mqtt_handler.hpp"
#include "mediumware.h"

#define TAG "[MQTT SERVICE]"

#define NO_NAME "No name"

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define MQTT_TOPIC(x) x
#define CONNECTION_MQTT_SEND_INFO MQTT_TOPIC("device_info")
#define MQTT_DEVICE_CHANGE CONNECTION_MQTT_SEND_INFO

namespace RetePulse
{   
    constexpr const char *general_key[] = {
        "device_name",
        "device_model",
        "last_updated",
        "sensor_type"
    };

    esp_err_t MqttMaintainer::start(const esp_mqtt_client_config_t &config)
    {
        /* Create binary semaphore for thread safety */
        this->mutex = xSemaphoreCreateBinaryStatic(&this->semaphore_buffer);
        if (this->mutex == NULL) {
            ESP_LOGE(TAG, "Could not initialize semaphore for MQTT service");
            return ESP_FAIL;
        }
        xSemaphoreGive(this->mutex);

        this->config = config; /* Store the MQTT client configuration */

        strncpy(this->name, this->config.credentials.client_id ? this->config.credentials.client_id : NO_NAME, STRING_SIZE - 1);
        strncpy(this->last_updated, __DATE__, STRING_SIZE - 1);
        strncpy(this->sensor_type, "uart", STRING_SIZE - 1);

        this->mqtt_device_map = hashmap_create(); /* Create a new hashmap for storing MQTT device subscriptions */

        this->checkDeviceName(this->config.credentials.client_id);
        if (this->mqtt_device_map != NULL) {
            esp_err_t con = setupMqttService(this->config.buffer.size, this->config.buffer.out_size);
            if (con == ESP_OK) {
                ESP_LOGI(TAG, "MQTT service initialized successfully");
                setup_cjson_pool(); /* Initialize cJSON pool for JSON handling */
                return con; // Return success if all steps are successful
            }
            #ifdef MQTT_O_DEBUG
            else {
                ESP_LOGE(TAG, "Failed to initialize MQTT service");
            }
            #endif
        }

        vSemaphoreDelete(this->mutex);
        this->mutex = NULL;
        return ESP_FAIL; // Return failure if any step fails
    }

    esp_err_t MqttMaintainer::stop()
    {
        esp_err_t ret = ESP_OK;

        if (xSemaphoreTake(this->mutex, portMAX_DELAY) == pdTRUE) {
            if (this->client != NULL) {
                esp_mqtt_client_unregister_event(this->client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqttEventHandlerHelper);

                esp_err_t stop_result = esp_mqtt_client_stop(this->client);
                if (stop_result != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to stop MQTT client: %s", esp_err_to_name(stop_result));
                    ret = stop_result; // Capture the error if stopping fails
                }

                esp_err_t destroy_result = esp_mqtt_client_destroy(this->client);
                if (destroy_result != ESP_OK) {
                    ESP_LOGE(TAG, "Failed to destroy MQTT client: %s", esp_err_to_name(destroy_result));
                    ret = destroy_result; // Capture the error if destroying fails
                }
                this->client = NULL;
            }
            xSemaphoreGive(this->mutex);

            return ret;
        }
        else {
            ESP_LOGE(TAG, "Could not get semaphore for MQTT service stop");
            return ESP_FAIL; // Return failure if semaphore could not be taken
        }
    }

    bool MqttMaintainer::addMqttClientSubscribe(
        const char *topic, 
        int qos,
        mqtt_event_data_action_t action
    ) {
        int id = esp_mqtt_client_subscribe(this->client, topic, qos);
        if (id < 0) {
            return false; // Subscription failed
        }
        else {
            return hashmap_put(this->mqtt_device_map, topic, this->convertFuncToIntptr(action));
        }
    }

    int MqttMaintainer::sendToMqttServiceSingle(char *const topic, char const *const key, const char *const data)
    {
        cjson_pool_reset(); // Reset pool before building new JSON tree

        cJSON *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, key, data);
    
        char json_buffer[128];
        bool success = cJSON_PrintPreallocated(root, json_buffer, sizeof(json_buffer), 0);
        if (success) {
            int len = strlen(json_buffer);
            if (len < 0) {
                ESP_LOGE(TAG, "Failed to create JSON payload");
                return -1;
            }
            return esp_mqtt_client_publish(this->client, topic, json_buffer, len, 1, 0);
        }
        else {
            return -2;
        }
    }

    int MqttMaintainer::sendToMqttServiceMultiple(char *const topic, const char**key, const char**data, int len)
    {
        cjson_pool_reset(); // Reset pool before building new JSON tree

        cJSON *root = cJSON_CreateObject();
        for (int i = 0; i < len; i++) {
            if (!key[i] || !data[i]) {
                ESP_LOGE(TAG, "Key or data is NULL at index %d", i);
                return -1; // Indicate failure
            }
            #ifdef MQTT_O_DEBUG
            ESP_LOGI(TAG, "Adding key: %s with data: %s", key[i], data[i]);
            #endif
            cJSON_AddStringToObject(root, key[i], data[i]);
        }
    
        char json_buffer[256];
        bool success = cJSON_PrintPreallocated(root, json_buffer, sizeof(json_buffer), 0);
        if (success) {
            int json_string_size = strlen(json_buffer);
            if (json_string_size < 0) {
                ESP_LOGE(TAG, "Failed to create JSON payload");
                return -1;
            }
            else {
                return esp_mqtt_client_publish(this->client, topic, json_buffer, json_string_size, 1, 0);
            }
        }
        else {
            return -2;
        }
    }

    void *MqttMaintainer::convertFuncToIntptr(mqtt_event_data_action_t action)
    {
        return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(action));
    }

    mqtt_event_data_action_t MqttMaintainer::convertIntptrToFunc(void *ptr)
    {
        return reinterpret_cast<mqtt_event_data_action_t>(reinterpret_cast<uintptr_t>(ptr));
    }

    void MqttMaintainer::mqttEventHandlerHelper(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data)
    {
        MqttMaintainer *mqtt_service = static_cast<MqttMaintainer*>(handler_args);
        mqtt_service->mqttEventHandler(handler_args, base, event_id, event_data);
    }

    esp_err_t MqttMaintainer::setupMqttService(int &buffer_size, int &out_size)
    {
        if (xSemaphoreTake(this->mutex, portMAX_DELAY) != pdFALSE) {
            /* Enforce minimum buffer sizes */
            if (buffer_size < 1024) {
                buffer_size = 1024; /* Set minimum buffer size */
            }

            if (out_size < 512) {
                out_size = 512; /* Set minimum output size */
            }
            /* Configure the MQTT client with the broker URI */
            /* Initialize the MQTT client */
            this->client = esp_mqtt_client_init(&this->config);
            if (this->client != NULL) {
                /* Register the event handler for all MQTT events */
                esp_mqtt_client_register_event(this->client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqttEventHandlerHelper, this);
                /* Start the MQTT client (runs on core 0) */
                esp_err_t ca = esp_mqtt_client_start(this->client);
                if (ca == ESP_OK) {
                    ESP_LOGI(TAG, "Has been successfully been set");
                    xSemaphoreGive(this->mutex);
                    return ca;
                }
                #ifdef MQTT_O_DEBUG
                else {
                    ESP_LOGE(TAG, "Failed to start MQTT client: %s", esp_err_to_name(ca));
                }
                #endif
            }
            #ifdef MQTT_O_DEBUG
            else {
                ESP_LOGE(TAG, "Failed to initialize MQTT client");
            }
            #endif
            xSemaphoreGive(this->mutex);
        }
        #ifdef MQTT_O_DEBUG
        else {
            ESP_LOGE(TAG, "Could not get semaphore for MQTT service setup");
        }
        #endif
        return ESP_FAIL; // Return failure if any step fails
    }

    void MqttMaintainer::mqttDataRecievedHandler(mqtt_data_package_t *package)
    {
        char key[32];
        int key_len = min(package->event->topic_len, sizeof(key) - 1);
        strncpy(key, package->event->topic, key_len);
        key[key_len] = '\0';  // Ensure null-termination

        mqtt_event_data_action_t action = this->convertIntptrToFunc(hashmap_get(this->mqtt_device_map, key));
        if (action) {
            action(package);
        }
        #ifdef MQTT_O_DEBUG
        else {
            ESP_LOGW(TAG, "No action defined for topic: %.*s", package->event->topic_len, package->event->topic);
        }
        #endif
    }

    int MqttMaintainer::sendConnectionInfo() {
        char general_info[4][MqttMaintainer::STRING_SIZE];
        const char *info_ptrs[4];

        int device_name_length = min(strlen(this->name), MqttMaintainer::STRING_SIZE - 1);
        strncpy(general_info[0], this->name, device_name_length);
        general_info[0][device_name_length] = '\0';
        info_ptrs[0] = general_info[0];

        int device_model_length = min(strlen(CONFIG_IDF_TARGET), MqttMaintainer::STRING_SIZE - 1);
        strncpy(general_info[1], CONFIG_IDF_TARGET, device_model_length);
        general_info[1][device_model_length] = '\0';
        info_ptrs[1] = general_info[1];

        int last_updated_length = min(strlen(this->last_updated), MqttMaintainer::STRING_SIZE - 1);
        strncpy(general_info[2], this->last_updated, last_updated_length);
        general_info[2][last_updated_length] = '\0';
        info_ptrs[2] = general_info[2];

        int sensor_type_length = min(strlen(this->sensor_type), MqttMaintainer::STRING_SIZE - 1);
        strncpy(general_info[3], this->sensor_type, sensor_type_length);
        general_info[3][sensor_type_length] = '\0';
        info_ptrs[3] = general_info[3];

        return this->sendToMqttServiceMultiple(
            CONNECTION_MQTT_SEND_INFO, 
            (const char **)general_key, 
            (const char **)info_ptrs, 
            4
        );
    }

    static void turnoff_led(mqtt_data_package_t *package)
    {
        esp_mqtt_event_handle_t event = package->event;
        (void)event; // Suppress unused variable warning
    
        // Extract LED status
        char *led_status = get_cjson_string(package->json, "led_status");
        if (led_status == NULL) {
            ESP_LOGE(TAG, "LED status not found in data");
            return;
        }
    }

    static void ota_handle(mqtt_data_package_t *package) 
    {
        char *data = package->event->data;
        size_t data_len = package->event->data_len;

        (void)data; // Suppress unused variable warning
        (void)data_len; // Suppress unused variable warning

        //if (strncmp(data, "update", data_len) == 0) {
        //    xTaskCreate(&ota_task, "ota_task", 8192, NULL, 5, NULL);
        //}
    }

    static void control_handle(mqtt_data_package_t *package)
    {
        char *data = package->event->data;
        size_t data_len = package->event->data_len;

        (void)data; // Suppress unused variable warning
        (void)data_len; // Suppress unused variable warning
        char *action_type = get_cjson_string(package->json, "action");
        if (strcmp(action_type, "reconfigure") == 0) {
            MqttMaintainer::mqttReconfigure(package->handler);
        }
        else if (strcmp(action_type, "gpio") == 0) {
            char *option = get_cjson_string(package->json, "gpio");
            if (strcmp(option, "config") == 0) {
                configure_gpio(package->json);~
            }
            else {
                set_gpio_state(package->json);
            }
        }
        else if (strcmp(action_type, "ota_update") == 0) {
            ota_handle(package);
        }
    }

    void MqttMaintainer::mqttReconfigure(MqttMaintainer *self) 
    {
        self->reconfigureMqttClient();
    }

    const char *MqttMaintainer::getName() const
    {
        return this->name;
    }

    static void setTopic(char *src, const char *topic, char *client_id, int total_size) {
        snprintf(src, total_size, "%s/%s", topic, client_id);
    }

    void MqttMaintainer::mqttConnectHandler()
    {
        char full_topic[64];
        setTopic(full_topic, MqttMaintainer::controlTopic, this->name, sizeof(full_topic));
        if (!this->addMqttClientSubscribe(full_topic, 0, control_handle)) {
            ESP_LOGE(TAG, "Failed to subscribe to topic: %s", full_topic);
            return;
        }

        if (this->sendConnectionInfo() > 0) {
            ESP_LOGI(TAG, "Connection info sent successfully");
        }
        else {
            ESP_LOGE(TAG, "Failed to send connection info");
        }
    }

    void MqttMaintainer::mqttReconnectHandler() 
    {
        esp_err_t err = esp_mqtt_client_reconnect(this->client);
        if (err != ESP_OK) {
            ESP_LOGI(TAG, "Failed to reconnect");
        }
        else {
            ESP_LOGI(TAG, "Reconnected successfully");
        }
    }

    void MqttMaintainer::mqttDataHandler(esp_mqtt_event_handle_t event)
    {
        cJSON *root = check_cjson(event->data, event->data_len);
        if (root != NULL) {
            #ifdef MQTT_O_DEBUG
            ESP_LOGI(TAG, "MQTT_EVENT_DATA: Topic=%.*s, Data=%.*s",
                event->topic_len, event->topic, 
                event->data_len, event->data);
            #endif
            mqtt_data_package_t package = {
                .event = event,
                .json = root,
                .handler = this
            };
            this->mqttDataRecievedHandler(&package);
        }
    }

    void MqttMaintainer::mqttEventHandler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) 
    {
        if (xSemaphoreTake(this->mutex, portMAX_DELAY) == pdFALSE) {
            ESP_LOGE(TAG, "Could not get semaphore");
            return;
        }

        esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
        switch (event->event_id) {
            case MQTT_EVENT_CONNECTED:
                mqttConnectHandler();
                break;
            case MQTT_EVENT_DISCONNECTED:
                mqttReconnectHandler();
                break;
            case MQTT_EVENT_DATA:
                mqttDataHandler(event);
                break;
            default:
                /* Log other MQTT events */
                ESP_LOGI(TAG, "Other event id:%d", event->event_id);
                break;
        }

        xSemaphoreGive(this->mutex);
    }

    int MqttMaintainer::checkDeviceName(const char *new_name)
    {
        int name_length;
        const char *n;
        if (new_name) {
            name_length = min(strlen(new_name) + 1, sizeof(this->name));
            n = new_name; // Use provided name
        } 
        else {
            name_length = min(sizeof(NO_NAME), sizeof(this->name));
            n = NO_NAME; // Use default name if new_name is NULL
        }
        strncpy(this->name, n, name_length);

        return 0; // Indicate success
    }

    void MqttMaintainer::reconfigureMqttClient()
    {
        this->sendConnectionInfo(); // Send connection info after reconfiguration
    }
}