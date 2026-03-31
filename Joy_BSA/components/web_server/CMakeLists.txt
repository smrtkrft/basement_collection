idf_component_register(
    SRCS "web_server.c" "api_handlers.c" "file_upload.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_http_server spiffs vfs ext_flash config_mgr audio_player led_ctrl wifi_mgr json
    EMBED_FILES "${CMAKE_SOURCE_DIR}/web_frontend/index.html"
)
