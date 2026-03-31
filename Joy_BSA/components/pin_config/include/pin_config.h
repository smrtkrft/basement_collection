#pragma once

#include "driver/gpio.h"

// External SPI Flash (SPI2_HOST)
#define EXT_FLASH_CS     GPIO_NUM_3
#define EXT_FLASH_MISO   GPIO_NUM_0
#define EXT_FLASH_MOSI   GPIO_NUM_4
#define EXT_FLASH_CLK    GPIO_NUM_6

// I2S Audio (MAX98357A)
#define I2S_BCLK         GPIO_NUM_1
#define I2S_WS           GPIO_NUM_2
#define I2S_DOUT         GPIO_NUM_5

// Button
#define BUTTON_GPIO      GPIO_NUM_7

// LEDs (PWM via LEDC)
#define LED_1_GPIO       GPIO_NUM_8
#define LED_2_GPIO       GPIO_NUM_9
#define LED_3_GPIO       GPIO_NUM_10
