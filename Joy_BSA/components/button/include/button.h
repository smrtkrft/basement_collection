#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef void (*button_callback_t)(void);

esp_err_t button_init(button_callback_t on_press);
