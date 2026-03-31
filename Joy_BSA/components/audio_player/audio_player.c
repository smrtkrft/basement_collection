#include "audio_player.h"
#include "wav_decoder.h"
#include "pin_config.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/i2s_std.h"

#include <string.h>

static const char *TAG = "audio_player";

#define AUDIO_BUF_SIZE     4096
#define AUDIO_TASK_STACK   8192
#define AUDIO_TASK_PRIO    5

static i2s_chan_handle_t s_i2s_tx = NULL;
static TaskHandle_t s_play_task = NULL;
static volatile bool s_playing = false;
static volatile bool s_stop_request = false;
static uint8_t s_volume = 80; // 0-100
static SemaphoreHandle_t s_mutex = NULL;

static char s_current_file[128] = {0};

static void audio_play_task(void *arg)
{
    FILE *f = fopen(s_current_file, "rb");
    if (f == NULL) {
        ESP_LOGE(TAG, "Cannot open file: %s", s_current_file);
        s_playing = false;
        s_play_task = NULL;
        vTaskDelete(NULL);
        return;
    }

    wav_header_info_t wav_info;
    esp_err_t ret = wav_decoder_parse_header(f, &wav_info);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Invalid WAV file");
        fclose(f);
        s_playing = false;
        s_play_task = NULL;
        vTaskDelete(NULL);
        return;
    }

    // Reconfigure I2S clock for this file's sample rate
    i2s_std_clk_config_t clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(wav_info.sample_rate);
    i2s_channel_disable(s_i2s_tx);
    i2s_channel_reconfig_std_clock(s_i2s_tx, &clk_cfg);

    // Configure slot based on channels
    i2s_std_slot_config_t slot_cfg;
    if (wav_info.channels == 1) {
        slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
    } else {
        slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
    }
    i2s_channel_reconfig_std_slot(s_i2s_tx, &slot_cfg);
    i2s_channel_enable(s_i2s_tx);

    uint8_t *buf = malloc(AUDIO_BUF_SIZE);
    if (buf == NULL) {
        ESP_LOGE(TAG, "Failed to allocate audio buffer");
        fclose(f);
        s_playing = false;
        s_play_task = NULL;
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "Playing: %s", s_current_file);

    uint32_t bytes_remaining = wav_info.data_size;
    size_t bytes_written;

    while (bytes_remaining > 0 && !s_stop_request) {
        size_t to_read = (bytes_remaining < AUDIO_BUF_SIZE) ? bytes_remaining : AUDIO_BUF_SIZE;
        size_t read = fread(buf, 1, to_read, f);
        if (read == 0) break;

        // Software volume control: scale 16-bit PCM samples
        if (s_volume < 100) {
            int16_t *samples = (int16_t *)buf;
            size_t num_samples = read / 2;
            for (size_t i = 0; i < num_samples; i++) {
                samples[i] = (int16_t)((int32_t)samples[i] * s_volume / 100);
            }
        }

        i2s_channel_write(s_i2s_tx, buf, read, &bytes_written, pdMS_TO_TICKS(1000));
        bytes_remaining -= read;
    }

    free(buf);
    fclose(f);

    // Flush remaining data with silence
    uint8_t silence[256] = {0};
    i2s_channel_write(s_i2s_tx, silence, sizeof(silence), &bytes_written, pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "Playback finished");

    s_playing = false;
    s_stop_request = false;
    s_play_task = NULL;
    vTaskDelete(NULL);
}

esp_err_t audio_player_init(void)
{
    s_mutex = xSemaphoreCreateMutex();

    // Create I2S channel
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.dma_desc_num = 8;
    chan_cfg.dma_frame_num = 256;

    esp_err_t ret = i2s_new_channel(&chan_cfg, &s_i2s_tx, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2S channel: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize with default config (44100 Hz, 16-bit, stereo)
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(44100),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCLK,
            .ws = I2S_WS,
            .dout = I2S_DOUT,
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    ret = i2s_channel_init_std_mode(s_i2s_tx, &std_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init I2S std mode: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Audio player initialized (I2S on BCLK=%d, WS=%d, DOUT=%d)",
             I2S_BCLK, I2S_WS, I2S_DOUT);
    return ESP_OK;
}

esp_err_t audio_player_play(const char *filepath)
{
    if (filepath == NULL) return ESP_ERR_INVALID_ARG;

    xSemaphoreTake(s_mutex, portMAX_DELAY);

    // Stop current playback if any
    if (s_playing && s_play_task != NULL) {
        s_stop_request = true;
        // Wait for task to finish
        int timeout = 50;
        while (s_playing && timeout > 0) {
            xSemaphoreGive(s_mutex);
            vTaskDelay(pdMS_TO_TICKS(20));
            xSemaphoreTake(s_mutex, portMAX_DELAY);
            timeout--;
        }
    }

    strncpy(s_current_file, filepath, sizeof(s_current_file) - 1);
    s_current_file[sizeof(s_current_file) - 1] = '\0';
    s_playing = true;
    s_stop_request = false;

    BaseType_t res = xTaskCreate(audio_play_task, "audio_play", AUDIO_TASK_STACK, NULL, AUDIO_TASK_PRIO, &s_play_task);
    xSemaphoreGive(s_mutex);

    if (res != pdPASS) {
        ESP_LOGE(TAG, "Failed to create audio task");
        s_playing = false;
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t audio_player_stop(void)
{
    if (s_playing) {
        s_stop_request = true;
        int timeout = 50;
        while (s_playing && timeout > 0) {
            vTaskDelay(pdMS_TO_TICKS(20));
            timeout--;
        }
    }
    return ESP_OK;
}

bool audio_player_is_playing(void)
{
    return s_playing;
}

esp_err_t audio_player_set_volume(uint8_t vol)
{
    if (vol > 100) vol = 100;
    s_volume = vol;
    return ESP_OK;
}
