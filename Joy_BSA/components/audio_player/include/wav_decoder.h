#pragma once

#include "esp_err.h"
#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint16_t channels;
    uint32_t sample_rate;
    uint16_t bits_per_sample;
    uint32_t data_size;
} wav_header_info_t;

esp_err_t wav_decoder_parse_header(FILE *f, wav_header_info_t *info);
