#pragma once

#include "driver/gpio.h"

// Xiao ESP32-C6 pin mapping: D-pin -> GPIO
// D0=GPIO0, D1=GPIO1, D2=GPIO2, D3=GPIO21, D4=GPIO22,
// D5=GPIO23, D6=GPIO16, D7=GPIO17, D8=GPIO19, D9=GPIO20, D10=GPIO18

// External SPI Flash (SPI2_HOST)
#define EXT_FLASH_CS     GPIO_NUM_21   // D3
#define EXT_FLASH_MISO   GPIO_NUM_0    // D0
#define EXT_FLASH_MOSI   GPIO_NUM_22   // D4
#define EXT_FLASH_CLK    GPIO_NUM_16   // D6

// I2S Audio (MAX98357A)
#define I2S_BCLK         GPIO_NUM_1    // D1
#define I2S_WS           GPIO_NUM_2    // D2
#define I2S_DOUT         GPIO_NUM_23   // D5

// LEDs (PWM via LEDC)
#define SWORD_LED_GPIO     GPIO_NUM_17   // D7
#define ARMOR_LED_GPIO     GPIO_NUM_19   // D8
#define RESERVED_LED_GPIO  GPIO_NUM_20   // D9

// Button
#define BUTTON_GPIO        GPIO_NUM_18   // D10
