#include "config_mgr.h"

#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

#include <stdlib.h>
#include <string.h>

static const char *TAG = "config_mgr";

esp_err_t config_mgr_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition truncated, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "Config manager initialized");
    return ESP_OK;
}

char *config_mgr_get_str(const char *key)
{
    nvs_handle_t handle;
    esp_err_t ret = nvs_open(CONFIG_NAMESPACE, NVS_READONLY, &handle);
    if (ret != ESP_OK) {
        return NULL;
    }

    size_t len = 0;
    ret = nvs_get_str(handle, key, NULL, &len);
    if (ret != ESP_OK || len == 0) {
        nvs_close(handle);
        return NULL;
    }

    char *value = malloc(len);
    if (value == NULL) {
        nvs_close(handle);
        return NULL;
    }

    ret = nvs_get_str(handle, key, value, &len);
    nvs_close(handle);

    if (ret != ESP_OK) {
        free(value);
        return NULL;
    }

    return value;
}

esp_err_t config_mgr_set_str(const char *key, const char *value)
{
    nvs_handle_t handle;
    esp_err_t ret = nvs_open(CONFIG_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = nvs_set_str(handle, key, value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set key '%s': %s", key, esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }

    ret = nvs_commit(handle);
    nvs_close(handle);
    return ret;
}

int32_t config_mgr_get_i32(const char *key, int32_t default_val)
{
    nvs_handle_t handle;
    esp_err_t ret = nvs_open(CONFIG_NAMESPACE, NVS_READONLY, &handle);
    if (ret != ESP_OK) {
        return default_val;
    }

    int32_t value;
    ret = nvs_get_i32(handle, key, &value);
    nvs_close(handle);

    if (ret != ESP_OK) {
        return default_val;
    }
    return value;
}

esp_err_t config_mgr_set_i32(const char *key, int32_t value)
{
    nvs_handle_t handle;
    esp_err_t ret = nvs_open(CONFIG_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = nvs_set_i32(handle, key, value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set key '%s': %s", key, esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }

    ret = nvs_commit(handle);
    nvs_close(handle);
    return ret;
}

esp_err_t config_mgr_erase(const char *key)
{
    nvs_handle_t handle;
    esp_err_t ret = nvs_open(CONFIG_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = nvs_erase_key(handle, key);
    if (ret == ESP_OK) {
        nvs_commit(handle);
    }
    nvs_close(handle);
    return ret;
}
