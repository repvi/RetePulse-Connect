set(SOURCES
    "hashmap.c"
    "mqtt_handler.cpp"
    "mqtt_operation.cpp"
    "ota_operation.c"
    "parsing.c"
    "wifi_handler.cpp"
    "wifi_operation.cpp"
)

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "include" "."
    REQUIRES 
        esp_wifi 
        esp_event 
        esp_system 
        nvs_flash
        mqtt
        driver
        json
        
    PRIV_REQUIRES 
        freertos 
        esp_http_client 
        esp_https_ota 
        esp_netif 
)

target_compile_definitions(
    ${COMPONENT_LIB}

    PRIVATE
        MQTT_O_DEBUG
)