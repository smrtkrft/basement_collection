idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES ext_flash config_mgr button led_ctrl audio_player wifi_mgr web_server api_client pin_config
)
