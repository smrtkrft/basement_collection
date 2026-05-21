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
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "cJSON.h"

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
        char filepath[280];
        snprintf(filepath, sizeof(filepath), "%s/%s", EXT_FLASH_AUDIO_DIR, audio_file);
        audio_player_play(filepath);
        free(audio_file);
    } else {
        ESP_LOGW(TAG, "No audio file selected");
    }

    // Start LED effect
    led_ctrl_set_effect(LED_EFFECT_CHASE);

    // Trigger every saved multi-API config (NVS key "apicfgs" — JSON array of
    // objects with {name, url, method, headers, body}). If nothing is configured,
    // fall back to the legacy single-config keys (api_url/api_method/...).
    char *stored = config_mgr_get_str("apicfgs");
    if (stored != NULL) {
        cJSON *arr = cJSON_Parse(stored);
        free(stored);
        if (arr != NULL && cJSON_IsArray(arr)) {
            int n = cJSON_GetArraySize(arr);
            for (int i = 0; i < n; i++) {
                cJSON *item = cJSON_GetArrayItem(arr, i);
                if (!cJSON_IsObject(item)) continue;
                cJSON *url = cJSON_GetObjectItem(item, "url");
                if (!cJSON_IsString(url) || strlen(url->valuestring) == 0) continue;
                cJSON *method = cJSON_GetObjectItem(item, "method");
                cJSON *headers = cJSON_GetObjectItem(item, "headers");
                cJSON *body = cJSON_GetObjectItem(item, "body");
                api_request_t req = {
                    .url = url->valuestring,
                    .method = cJSON_IsString(method) ? method->valuestring : "GET",
                    .headers = cJSON_IsString(headers) ? headers->valuestring : NULL,
                    .body = cJSON_IsString(body) ? body->valuestring : NULL,
                };
                api_client_send(&req);
            }
        }
        cJSON_Delete(arr);
        return;
    }

    // Legacy fallback: single API config in dedicated NVS keys.
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

    // External flash is OPTIONAL for bringup/debug. If it fails (chip dead,
    // bad solder, etc.) we still want the device to come up far enough to
    // start WiFi AP and the web UI, so the failure can be diagnosed remotely.
    // Audio playback is unavailable in this degraded mode (no filesystem
    // for the WAV files), but everything else works.
    bool ext_flash_ok = (ext_flash_init() == ESP_OK);
    if (ext_flash_ok) {
        size_t free_space = ext_flash_get_free_space();
        size_t total_space = ext_flash_get_total_space();
        ESP_LOGI(TAG, "External flash: %zu / %zu bytes free", free_space, total_space);
    } else {
        ESP_LOGW(TAG, "External flash unavailable — running in degraded mode (no audio)");
    }

    // Phase 2: Audio (I2S init is independent of ext_flash; playback will
    // simply fail to open files if ext_flash didn't mount.)
    ESP_ERROR_CHECK(audio_player_init());

    // Phase 3: Input/Output
    ESP_ERROR_CHECK(button_init(on_button_press));
    ESP_ERROR_CHECK(led_ctrl_init());
    ESP_ERROR_CHECK(api_client_init());

    // Phase 4: Network
    ESP_ERROR_CHECK(wifi_mgr_init());

    ESP_LOGI(TAG, "Device: BSA-%s", wifi_mgr_get_device_id());

    // Phase 5: Web interface
    ESP_ERROR_CHECK(web_server_init());

    ESP_LOGI(TAG, "System ready!");
}
