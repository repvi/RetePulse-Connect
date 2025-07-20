#include "mediumware.h"
#include "string.h"
#include "esp_system.h"
#include "esp_log.h"

#define min(x, y) ((x) < (y) ? (x) : (y))

#define TAG "[MEDIUM-WARE]"

void configure_gpio(cJSON *json)
{
    const char *output_str = "output";
    char *pin_num = get_cjson_string(json, "pin");
    char *pin_type = get_cjson_string(json, "state");
    if (pin_num && pin_type) {
        gpio_num_t pin = atoi(pin_num);

        int str_len = min(strlen(pin_type), strlen(output_str));
        if (strncmp(pin_type, output_str, str_len) == 0) {
            gpio_config_t io_conf = {
                .pin_bit_mask = (1ULL << pin),
                .mode = GPIO_MODE_OUTPUT,
                .pull_up_en = GPIO_PULLUP_DISABLE,
                .pull_down_en = GPIO_PULLDOWN_DISABLE,
                .intr_type = GPIO_INTR_DISABLE
            };
            gpio_config(&io_conf);
            gpio_set_level(pin, 0);

            printf("setting pin: %d", pin);
        }
        else {
            ESP_LOGE(TAG, "Could not find type for pin %s, type: %s", pin_num, pin_type);
        }
    }
}

void set_gpio_state(cJSON *json)
{
    char *gpio = get_cjson_string(json, "pin");
    char *state = get_cjson_string(json, "state");
    if (gpio && state) {
        gpio_num_t pin = atoi(gpio);
        if (strncmp(state, "on", 2) == 0) {
            ESP_LOGI(TAG, "Setting GPIO %d to HIGH", pin);
            gpio_set_level(pin, 1);
        }
        else if (strncmp(state, "off", 3) == 0) {
            ESP_LOGI(TAG, "Setting GPIO %d to LOW", pin);
            gpio_set_level(pin, 0);
        }
        else {
            ESP_LOGE(TAG, "Unknown GPIO state: %s", state);
        }
    }
    else {
        ESP_LOGE(TAG, "GPIO or state not specified in JSON");
    }
}
