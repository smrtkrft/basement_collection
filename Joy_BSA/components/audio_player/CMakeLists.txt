idf_component_register(
    SRCS "audio_player.c" "wav_decoder.c"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_driver_i2s pin_config
)
