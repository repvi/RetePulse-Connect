#include "mediumware.h"
#include "string.h"
#include "esp_system.h"
#include "esp_log.h"

#define TAG "[MEDIUM-WARE]"

void configure_gpio(cJSON *json)
{
    char *pin_num = get_cjson_string(json, "pin");
    char *pin_type = get_cjson_string(json, "type");
    if (pin_num && pin_type) {
        gpio_num_t pin = atoi(pin_num);
        
        if (strcpy(pin_type, "output") == 0) {
            gpio_config_t io_conf = {
                .pin_bit_mask = (1ULL << pin),
                .mode = GPIO_MODE_OUTPUT,
                .pull_up_en = GPIO_PULLUP_DISABLE,
                .pull_down_en = GPIO_PULLDOWN_DISABLE,
                .intr_type = GPIO_INTR_DISABLE
            };
            gpio_config(&io_conf);
            gpio_set_level(pin, 0);
        }
        else {
            ESP_LOGE(TAG, "Could not find type");
        }
    }
}

void set_gpio_state(cJSON *json)
{
    char *gpio = get_cjson_string(json, "pin");
    char *state = get_cjson_string(json, "state");
    if (gpio && state) {
        gpio_num_t pin = atoi(gpio);
        if (strcmp(state, "on") == 0) {
            gpio_set_level(pin, 0); 
        }
        else if (strcmp(state, "off") == 0) {
            gpio_set_level(pin, 0);
        }
    }
}
