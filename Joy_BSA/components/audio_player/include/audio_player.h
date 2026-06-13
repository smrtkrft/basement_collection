#pragma once

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

esp_err_t audio_player_init(void);
esp_err_t audio_player_play(const char *filepath);
esp_err_t audio_player_stop(void);
bool audio_player_is_playing(void);
esp_err_t audio_player_set_volume(uint8_t vol);

// Diagnostic: play a 440 Hz sine tone for `duration_ms` ms via I2S directly,
// bypassing the filesystem and WAV decoder. Use to verify the I2S+amp chain
// independent of audio storage.
esp_err_t audio_player_play_test_tone(uint32_t duration_ms);
