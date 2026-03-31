#pragma once

#include "esp_err.h"
#include <stdbool.h>

esp_err_t wifi_mgr_init(void);
esp_err_t wifi_mgr_start_ap(void);
esp_err_t wifi_mgr_start_sta(const char *ssid, const char *password);
bool wifi_mgr_is_connected(void);
esp_err_t wifi_mgr_get_ip(char *buf, size_t buf_len);
