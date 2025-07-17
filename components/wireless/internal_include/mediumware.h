#pragma once

#include "parsing.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

void configure_gpio(cJSON *json);

void set_gpio_state(cJSON *json);

#ifdef __cplusplus
}   
#endif