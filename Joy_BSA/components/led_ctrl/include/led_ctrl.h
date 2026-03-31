#pragma once

#include "esp_err.h"
#include <stdint.h>

typedef enum {
    LED_EFFECT_OFF = 0,
    LED_EFFECT_SOLID,
    LED_EFFECT_BLINK,
    LED_EFFECT_BREATHE,
    LED_EFFECT_CHASE,
} led_effect_t;

esp_err_t led_ctrl_init(void);
esp_err_t led_ctrl_set_effect(led_effect_t effect);
esp_err_t led_ctrl_set_brightness(uint8_t brightness);
esp_err_t led_ctrl_set_speed(uint8_t speed);
esp_err_t led_ctrl_stop(void);
