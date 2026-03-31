#pragma once

#include "esp_err.h"
#include <stdint.h>

#define CONFIG_NAMESPACE "joy_bsa"

/**
 * Initialize NVS and config manager.
 */
esp_err_t config_mgr_init(void);

/**
 * Get a string value from NVS. Caller must free the returned pointer.
 * Returns NULL if key not found.
 */
char *config_mgr_get_str(const char *key);

/**
 * Set a string value in NVS.
 */
esp_err_t config_mgr_set_str(const char *key, const char *value);

/**
 * Get an integer value from NVS. Returns default_val if key not found.
 */
int32_t config_mgr_get_i32(const char *key, int32_t default_val);

/**
 * Set an integer value in NVS.
 */
esp_err_t config_mgr_set_i32(const char *key, int32_t value);

/**
 * Erase a key from NVS.
 */
esp_err_t config_mgr_erase(const char *key);
