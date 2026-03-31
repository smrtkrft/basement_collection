#include "ext_flash.h"
#include "pin_config.h"

#include "esp_log.h"
#include "esp_flash.h"
#include "esp_flash_spi_init.h"
#include "esp_partition.h"
#include "esp_vfs_fat.h"
#include "driver/spi_common.h"

#include <string.h>
#include <sys/stat.h>

static const char *TAG = "ext_flash";

static esp_flash_t *s_ext_flash = NULL;
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;
static const esp_partition_t *s_fat_partition = NULL;

esp_err_t ext_flash_init(void)
{
    ESP_LOGI(TAG, "Initializing external SPI flash...");

    // Configure SPI bus
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

    // Configure external flash device
    esp_flash_spi_device_config_t dev_cfg = {
        .host_id = SPI2_HOST,
        .cs_id = 0,
        .cs_io_num = EXT_FLASH_CS,
        .io_mode = SPI_FLASH_DIO,
        .freq_mhz = 40,
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
        .max_files = 5,
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

    // Create audio directory if it doesn't exist
    struct stat st;
    if (stat(EXT_FLASH_AUDIO_DIR, &st) != 0) {
        mkdir(EXT_FLASH_AUDIO_DIR, 0755);
        ESP_LOGI(TAG, "Created audio directory: %s", EXT_FLASH_AUDIO_DIR);
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
        .max_files = 5,
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

    // Recreate audio directory
    mkdir(EXT_FLASH_AUDIO_DIR, 0755);

    ESP_LOGI(TAG, "Format complete, remounted at %s", EXT_FLASH_MOUNT_POINT);
    return ESP_OK;
}

size_t ext_flash_get_free_space(void)
{
    FATFS *fs;
    DWORD free_clusters;
    FRESULT res = f_getfree("0:", &free_clusters, &fs);
    if (res != FR_OK) {
        ESP_LOGE(TAG, "Failed to get free space: %d", res);
        return 0;
    }
    size_t free_bytes = (size_t)free_clusters * fs->csize * 512;
    return free_bytes;
}

size_t ext_flash_get_total_space(void)
{
    FATFS *fs;
    DWORD free_clusters;
    FRESULT res = f_getfree("0:", &free_clusters, &fs);
    if (res != FR_OK) {
        ESP_LOGE(TAG, "Failed to get total space: %d", res);
        return 0;
    }
    size_t total_bytes = (size_t)(fs->n_fatent - 2) * fs->csize * 512;
    return total_bytes;
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
