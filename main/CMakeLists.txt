idf_component_register(
    SRCS 
        "main.c"
    INCLUDE_DIRS "."
    REQUIRES 
        esp_wifi 
        esp_event 
        esp_system 
        nvs_flash
        freertos
        esp_http_client
        esp_https_ota
        esp_netif
        mqtt
        wireless
)
