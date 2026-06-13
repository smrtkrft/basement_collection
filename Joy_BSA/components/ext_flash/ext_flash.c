#include "ext_flash.h"
#include "pin_config.h"

#include "esp_log.h"
#include "esp_flash.h"
#include "esp_flash_spi_init.h"
#include "esp_partition.h"
#include "esp_vfs_fat.h"
#include "driver/spi_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

static const char *TAG = "ext_flash";

static esp_flash_t *s_ext_flash = NULL;
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;
static const esp_partition_t *s_fat_partition = NULL;

esp_err_t ext_flash_init(void)
{
    ESP_LOGI(TAG, "Initializing external SPI flash...");

    spi_bus_config_t bus_cfg = {
        .miso_io_num = EXT_FLASH_MISO,
        .mosi_io_num = EXT_FLASH_MOSI,
        .sclk_io_num = EXT_FLASH_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 64 * 1024,
    };

    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return ret;
    }

    // Brief settle delay so Vcc is stable before the chip's first command.
    vTaskDelay(pdMS_TO_TICKS(50));

    esp_flash_spi_device_config_t dev_cfg = {
        .host_id = SPI2_HOST,
        .cs_id = 0,
        .cs_io_num = EXT_FLASH_CS,
        .io_mode = SPI_FLASH_FASTRD,
        .freq_mhz = 5,
    };

    ret = spi_bus_add_flash_device(&s_ext_flash, &dev_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add flash device: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_flash_init(s_ext_flash);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize flash: %s", esp_err_to_name(ret));
        return ret;
    }

    uint32_t flash_size;
    ret = esp_flash_get_size(s_ext_flash, &flash_size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get flash size: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "External flash size: %lu MB", flash_size / (1024 * 1024));

    // Register the external flash as a partition
    ret = esp_partition_register_external(
        s_ext_flash,
        0,              // offset
        flash_size,     // size (entire flash)
        "ext_fat",      // label
        ESP_PARTITION_TYPE_DATA,
        ESP_PARTITION_SUBTYPE_DATA_FAT,
        &s_fat_partition
    );
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register external partition: %s", esp_err_to_name(ret));
        return ret;
    }

    // Mount FAT filesystem
    const esp_vfs_fat_mount_config_t mount_cfg = {
        .format_if_mount_failed = true,
        .max_files = 10,
        .allocation_unit_size = 4096,
    };

    ret = esp_vfs_fat_spiflash_mount_rw_wl(
        EXT_FLASH_MOUNT_POINT,
        "ext_fat",
        &mount_cfg,
        &s_wl_handle
    );
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FAT: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "External flash mounted at %s", EXT_FLASH_MOUNT_POINT);

    // Verify write access - if read-only, format and remount
    char test_path[64];
    snprintf(test_path, sizeof(test_path), "%s/wtest.tmp", EXT_FLASH_MOUNT_POINT);
    FILE *tf = fopen(test_path, "w");
    if (tf) {
        fclose(tf);
        remove(test_path);
        ESP_LOGI(TAG, "Write test: OK");
    } else {
        ESP_LOGW(TAG, "Write test failed, formatting flash...");

        // Unmount
        esp_vfs_fat_spiflash_unmount_rw_wl(EXT_FLASH_MOUNT_POINT, s_wl_handle);
        s_wl_handle = WL_INVALID_HANDLE;

        // Erase entire flash chip
        ret = esp_flash_erase_chip(s_ext_flash);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Flash erase failed: %s", esp_err_to_name(ret));
            return ret;
        }
        ESP_LOGI(TAG, "Flash erased");

        // Remount (will auto-format since erased)
        ret = esp_vfs_fat_spiflash_mount_rw_wl(
            EXT_FLASH_MOUNT_POINT,
            "ext_fat",
            &mount_cfg,
            &s_wl_handle
        );
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Remount after format failed: %s", esp_err_to_name(ret));
            return ret;
        }
        ESP_LOGI(TAG, "Flash formatted and remounted");

        // Verify write works now
        tf = fopen(test_path, "w");
        if (tf) {
            fputs("ok", tf);
            fclose(tf);
            remove(test_path);
            ESP_LOGI(TAG, "Write test after format: OK");
        } else {
            ESP_LOGW(TAG, "Write test file failed (errno=%d), mkdir may still work", errno);
        }
    }

    // Create directories
    struct stat st;
    if (stat(EXT_FLASH_AUDIO_DIR, &st) != 0) {
        mkdir(EXT_FLASH_AUDIO_DIR, 0755);
        ESP_LOGI(TAG, "Created: %s", EXT_FLASH_AUDIO_DIR);
    }
    if (stat(EXT_FLASH_API_DIR, &st) != 0) {
        mkdir(EXT_FLASH_API_DIR, 0755);
        ESP_LOGI(TAG, "Created: %s", EXT_FLASH_API_DIR);
    }

    return ESP_OK;
}

esp_err_t ext_flash_format(void)
{
    ESP_LOGW(TAG, "Formatting external flash...");

    // Unmount first
    esp_vfs_fat_spiflash_unmount_rw_wl(EXT_FLASH_MOUNT_POINT, s_wl_handle);
    s_wl_handle = WL_INVALID_HANDLE;

    // Remount with format
    const esp_vfs_fat_mount_config_t mount_cfg = {
        .format_if_mount_failed = true,
        .max_files = 10,
        .allocation_unit_size = 4096,
    };

    // Erase flash before remounting
    esp_err_t ret = esp_flash_erase_chip(s_ext_flash);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to erase flash: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_vfs_fat_spiflash_mount_rw_wl(
        EXT_FLASH_MOUNT_POINT,
        "ext_fat",
        &mount_cfg,
        &s_wl_handle
    );
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to remount after format: %s", esp_err_to_name(ret));
        return ret;
    }

    // Recreate directories
    mkdir(EXT_FLASH_AUDIO_DIR, 0755);
    mkdir(EXT_FLASH_API_DIR, 0755);

    ESP_LOGI(TAG, "Format complete, remounted at %s", EXT_FLASH_MOUNT_POINT);
    return ESP_OK;
}

size_t ext_flash_get_free_space(void)
{
    uint64_t total = 0, free_bytes = 0;
    esp_err_t ret = esp_vfs_fat_info(EXT_FLASH_MOUNT_POINT, &total, &free_bytes);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get free space: %s", esp_err_to_name(ret));
        return 0;
    }
    return (size_t)free_bytes;
}

size_t ext_flash_get_total_space(void)
{
    uint64_t total = 0, free_bytes = 0;
    esp_err_t ret = esp_vfs_fat_info(EXT_FLASH_MOUNT_POINT, &total, &free_bytes);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get total space: %s", esp_err_to_name(ret));
        return 0;
    }
    return (size_t)total;
}

void ext_flash_deinit(void)
{
    if (s_wl_handle != WL_INVALID_HANDLE) {
        esp_vfs_fat_spiflash_unmount_rw_wl(EXT_FLASH_MOUNT_POINT, s_wl_handle);
        s_wl_handle = WL_INVALID_HANDLE;
    }
    if (s_ext_flash) {
        spi_bus_remove_flash_device(s_ext_flash);
        s_ext_flash = NULL;
    }
    spi_bus_free(SPI2_HOST);
    ESP_LOGI(TAG, "External flash deinitialized");
}
