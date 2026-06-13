#include "button.h"
#include "pin_config.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "button";

// Minimum window between accepted press events (microseconds). Filters out
// the few ms of contact bounce; short enough that quick taps are caught.
#define DEBOUNCE_US (30 * 1000)

static button_callback_t s_callback = NULL;
static QueueHandle_t s_event_queue = NULL;
static volatile int64_t s_last_press_us = 0;

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    int64_t now = esp_timer_get_time();
    if (now - s_last_press_us < DEBOUNCE_US) return;
    s_last_press_us = now;

    uint8_t event = 1;
    BaseType_t hpw = pdFALSE;
    xQueueSendFromISR(s_event_queue, &event, &hpw);
    if (hpw) portYIELD_FROM_ISR();
}

static void button_task(void *arg)
{
    uint8_t event;
    while (1) {
        if (xQueueReceive(s_event_queue, &event, portMAX_DELAY)) {
            ESP_LOGI(TAG, "Button press detected");
            if (s_callback) s_callback();
        }
    }
}

esp_err_t button_init(button_callback_t on_press)
{
    s_callback = on_press;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) return ret;

    // Stack must fit the on-press callback (HTTP, JSON, audio, LED).
    s_event_queue = xQueueCreate(4, sizeof(uint8_t));
    xTaskCreate(button_task, "button", 8192, NULL, 3, NULL);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_GPIO, gpio_isr_handler, NULL);

    ESP_LOGI(TAG, "Button initialized on GPIO%d", BUTTON_GPIO);
    return ESP_OK;
}
