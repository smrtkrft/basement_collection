#include "led_ctrl.h"
#include "pin_config.h"

#include "esp_log.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <math.h>

static const char *TAG = "led_ctrl";

#define NUM_LEDS       3
#define LEDC_FREQ_HZ   5000
#define LEDC_RESOLUTION LEDC_TIMER_8_BIT
#define LED_TASK_STACK  2048

static const gpio_num_t led_gpios[NUM_LEDS] = {LED_1_GPIO, LED_2_GPIO, LED_3_GPIO};
static const ledc_channel_t led_channels[NUM_LEDS] = {LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2};

static TaskHandle_t s_led_task = NULL;
static volatile led_effect_t s_current_effect = LED_EFFECT_OFF;
static volatile uint8_t s_brightness = 255;
static volatile uint8_t s_speed = 50; // 1-100

static void set_led_duty(int led, uint8_t duty)
{
    if (led >= 0 && led < NUM_LEDS) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, led_channels[led], duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, led_channels[led]);
    }
}

static void all_leds_off(void)
{
    for (int i = 0; i < NUM_LEDS; i++) {
        set_led_duty(i, 0);
    }
}

static uint32_t speed_to_delay(void)
{
    // speed 1=slow(500ms), 100=fast(20ms)
    return 520 - (s_speed * 5);
}

static void effect_solid(void)
{
    for (int i = 0; i < NUM_LEDS; i++) {
        set_led_duty(i, s_brightness);
    }
}

static void effect_blink(void)
{
    static bool on = false;
    on = !on;
    for (int i = 0; i < NUM_LEDS; i++) {
        set_led_duty(i, on ? s_brightness : 0);
    }
}

static void effect_breathe(uint32_t step)
{
    // Sine wave breathing
    float val = (sinf((float)step * 0.05f) + 1.0f) / 2.0f;
    uint8_t duty = (uint8_t)(val * s_brightness);
    for (int i = 0; i < NUM_LEDS; i++) {
        set_led_duty(i, duty);
    }
}

static void effect_chase(uint32_t step)
{
    int active = step % NUM_LEDS;
    for (int i = 0; i < NUM_LEDS; i++) {
        set_led_duty(i, (i == active) ? s_brightness : 0);
    }
}

static void led_task(void *arg)
{
    uint32_t step = 0;

    while (1) {
        led_effect_t effect = s_current_effect;

        switch (effect) {
        case LED_EFFECT_OFF:
            all_leds_off();
            vTaskDelay(pdMS_TO_TICKS(100));
            break;
        case LED_EFFECT_SOLID:
            effect_solid();
            vTaskDelay(pdMS_TO_TICKS(100));
            break;
        case LED_EFFECT_BLINK:
            effect_blink();
            vTaskDelay(pdMS_TO_TICKS(speed_to_delay()));
            break;
        case LED_EFFECT_BREATHE:
            effect_breathe(step);
            vTaskDelay(pdMS_TO_TICKS(20));
            break;
        case LED_EFFECT_CHASE:
            effect_chase(step);
            vTaskDelay(pdMS_TO_TICKS(speed_to_delay()));
            break;
        }
        step++;
    }
}

esp_err_t led_ctrl_init(void)
{
    // Configure LEDC timer
    ledc_timer_config_t timer_cfg = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_RESOLUTION,
        .freq_hz = LEDC_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    esp_err_t ret = ledc_timer_config(&timer_cfg);
    if (ret != ESP_OK) return ret;

    // Configure each LED channel
    for (int i = 0; i < NUM_LEDS; i++) {
        ledc_channel_config_t ch_cfg = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = led_channels[i],
            .timer_sel = LEDC_TIMER_0,
            .intr_type = LEDC_INTR_DISABLE,
            .gpio_num = led_gpios[i],
            .duty = 0,
            .hpoint = 0,
        };
        ret = ledc_channel_config(&ch_cfg);
        if (ret != ESP_OK) return ret;
    }

    // Create LED animation task
    xTaskCreate(led_task, "led_ctrl", LED_TASK_STACK, NULL, 2, &s_led_task);

    ESP_LOGI(TAG, "LED controller initialized (GPIO%d, GPIO%d, GPIO%d)",
             LED_1_GPIO, LED_2_GPIO, LED_3_GPIO);
    return ESP_OK;
}

esp_err_t led_ctrl_set_effect(led_effect_t effect)
{
    s_current_effect = effect;
    ESP_LOGI(TAG, "Effect set: %d", effect);
    return ESP_OK;
}

esp_err_t led_ctrl_set_brightness(uint8_t brightness)
{
    s_brightness = brightness;
    return ESP_OK;
}

esp_err_t led_ctrl_set_speed(uint8_t speed)
{
    if (speed < 1) speed = 1;
    if (speed > 100) speed = 100;
    s_speed = speed;
    return ESP_OK;
}

esp_err_t led_ctrl_stop(void)
{
    s_current_effect = LED_EFFECT_OFF;
    all_leds_off();
    return ESP_OK;
}
