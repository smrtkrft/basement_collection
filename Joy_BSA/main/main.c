/*
 * SmartKraft - Joy_BSA
 * github.com/smrtkrft/basement_collection/Joy_BSA
 * SEU - Emek Ulas
 *
 * ESP32-C6 Audio Player with External Flash & Web Interface
 * Built with ESP-IDF
 *
 * Hardware:
 * - Xiao ESP32-C6
 * - 16MB External SPI Flash (audio storage)
 * - MAX98357A I2S Audio Amplifier
 * - 1 Button (GPIO7)
 * - 3 LEDs (GPIO8, GPIO9, GPIO10)
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "pin_config.h"
#include "ext_flash.h"
#include "config_mgr.h"
#include "audio_player.h"
#include "button.h"
#include "led_ctrl.h"
#include "wifi_mgr.h"
#include "web_server.h"
#include "api_client.h"

static const char *TAG = "joy_bsa";

static void on_button_press(void)
{
    ESP_LOGI(TAG, "Button pressed!");

    // Get selected audio file from config
    char *audio_file = config_mgr_get_str("audio_file");
    if (audio_file != NULL) {
        char filepath[128];
        snprintf(filepath, sizeof(filepath), "%s/%s", EXT_FLASH_AUDIO_DIR, audio_file);
        audio_player_play(filepath);
        free(audio_file);
    } else {
        ESP_LOGW(TAG, "No audio file selected");
    }

    // Start LED effect
    led_ctrl_set_effect(LED_EFFECT_CHASE);

    // Trigger API call
    char *url = config_mgr_get_str("api_url");
    if (url != NULL) {
        char *method = config_mgr_get_str("api_method");
        char *headers = config_mgr_get_str("api_headers");
        char *body = config_mgr_get_str("api_body");

        api_request_t req = {
            .url = url,
            .method = method ? method : "GET",
            .headers = headers,
            .body = body,
        };
        api_client_send(&req);

        free(url);
        free(method);
        free(headers);
        free(body);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "SmartKraft - Joy_BSA");
    ESP_LOGI(TAG, "ESP32-C6 Audio Player");
    ESP_LOGI(TAG, "=================================");

    // Phase 1: Core initialization
    ESP_ERROR_CHECK(config_mgr_init());
    ESP_ERROR_CHECK(ext_flash_init());

    size_t free_space = ext_flash_get_free_space();
    size_t total_space = ext_flash_get_total_space();
    ESP_LOGI(TAG, "External flash: %zu / %zu bytes free", free_space, total_space);

    // Phase 2: Audio
    ESP_ERROR_CHECK(audio_player_init());

    // Phase 3: Input/Output
    ESP_ERROR_CHECK(button_init(on_button_press));
    ESP_ERROR_CHECK(led_ctrl_init());
    ESP_ERROR_CHECK(api_client_init());

    // Phase 4: Network
    ESP_ERROR_CHECK(wifi_mgr_init());

    // Phase 5: Web interface
    ESP_ERROR_CHECK(web_server_init());

    ESP_LOGI(TAG, "System ready!");
}
