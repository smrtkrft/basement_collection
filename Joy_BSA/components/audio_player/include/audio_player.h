#pragma once

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

esp_err_t audio_player_init(void);
esp_err_t audio_player_play(const char *filepath);
esp_err_t audio_player_stop(void);
bool audio_player_is_playing(void);
esp_err_t audio_player_set_volume(uint8_t vol);
