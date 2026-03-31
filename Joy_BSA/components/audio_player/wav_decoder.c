#include "wav_decoder.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "wav_decoder";

typedef struct __attribute__((packed)) {
    char     riff_tag[4];     // "RIFF"
    uint32_t riff_size;
    char     wave_tag[4];     // "WAVE"
    char     fmt_tag[4];      // "fmt "
    uint32_t fmt_size;
    uint16_t audio_format;    // 1 = PCM
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
} wav_header_t;

esp_err_t wav_decoder_parse_header(FILE *f, wav_header_info_t *info)
{
    if (f == NULL || info == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    fseek(f, 0, SEEK_SET);

    wav_header_t hdr;
    if (fread(&hdr, 1, sizeof(hdr), f) != sizeof(hdr)) {
        ESP_LOGE(TAG, "Failed to read WAV header");
        return ESP_ERR_INVALID_SIZE;
    }

    if (memcmp(hdr.riff_tag, "RIFF", 4) != 0 || memcmp(hdr.wave_tag, "WAVE", 4) != 0) {
        ESP_LOGE(TAG, "Not a valid WAV file");
        return ESP_ERR_INVALID_RESPONSE;
    }

    if (memcmp(hdr.fmt_tag, "fmt ", 4) != 0) {
        ESP_LOGE(TAG, "Missing fmt chunk");
        return ESP_ERR_INVALID_RESPONSE;
    }

    if (hdr.audio_format != 1) {
        ESP_LOGE(TAG, "Unsupported format: %d (only PCM supported)", hdr.audio_format);
        return ESP_ERR_NOT_SUPPORTED;
    }

    if (hdr.bits_per_sample != 16) {
        ESP_LOGE(TAG, "Unsupported bits per sample: %d (only 16-bit supported)", hdr.bits_per_sample);
        return ESP_ERR_NOT_SUPPORTED;
    }

    // Skip any extra fmt bytes
    if (hdr.fmt_size > 16) {
        fseek(f, hdr.fmt_size - 16, SEEK_CUR);
    }

    // Find the "data" chunk
    char chunk_id[4];
    uint32_t chunk_size;
    while (1) {
        if (fread(chunk_id, 1, 4, f) != 4 || fread(&chunk_size, 1, 4, f) != 4) {
            ESP_LOGE(TAG, "Data chunk not found");
            return ESP_ERR_NOT_FOUND;
        }
        if (memcmp(chunk_id, "data", 4) == 0) {
            break;
        }
        // Skip unknown chunk
        fseek(f, chunk_size, SEEK_CUR);
    }

    info->channels = hdr.channels;
    info->sample_rate = hdr.sample_rate;
    info->bits_per_sample = hdr.bits_per_sample;
    info->data_size = chunk_size;

    ESP_LOGI(TAG, "WAV: %lu Hz, %d-bit, %d ch, %lu bytes",
             info->sample_rate, info->bits_per_sample, info->channels, info->data_size);

    return ESP_OK;
}
