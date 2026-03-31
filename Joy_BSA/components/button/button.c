#include "button.h"
#include "pin_config.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "button";

#define DEBOUNCE_MS 200

static button_callback_t s_callback = NULL;
static esp_timer_handle_t s_debounce_timer = NULL;
static QueueHandle_t s_event_queue = NULL;
static volatile bool s_debounce_active = false;

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    if (!s_debounce_active) {
        s_debounce_active = true;
        esp_timer_start_once(s_debounce_timer, DEBOUNCE_MS * 1000);
    }
}

static void debounce_timer_cb(void *arg)
{
    // Check if button is still pressed (active low)
    if (gpio_get_level(BUTTON_GPIO) == 0) {
        uint8_t event = 1;
        xQueueSendFromISR(s_event_queue, &event, NULL);
    }
    s_debounce_active = false;
}

static void button_task(void *arg)
{
    uint8_t event;
    while (1) {
        if (xQueueReceive(s_event_queue, &event, portMAX_DELAY)) {
            ESP_LOGI(TAG, "Button press detected");
            if (s_callback) {
                s_callback();
            }
        }
    }
}

esp_err_t button_init(button_callback_t on_press)
{
    s_callback = on_press;

    // Configure GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) return ret;

    // Create debounce timer
    esp_timer_create_args_t timer_args = {
        .callback = debounce_timer_cb,
        .name = "btn_debounce",
    };
    ret = esp_timer_create(&timer_args, &s_debounce_timer);
    if (ret != ESP_OK) return ret;

    // Create event queue and task
    s_event_queue = xQueueCreate(4, sizeof(uint8_t));
    xTaskCreate(button_task, "button", 2048, NULL, 3, NULL);

    // Install ISR
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_GPIO, gpio_isr_handler, NULL);

    ESP_LOGI(TAG, "Button initialized on GPIO%d", BUTTON_GPIO);
    return ESP_OK;
}
