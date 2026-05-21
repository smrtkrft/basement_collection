#include "ext_flash.h"
#include "pin_config.h"

#include "esp_log.h"
#include "esp_flash.h"
#include "esp_flash_spi_init.h"
#include "esp_partition.h"
#include "esp_vfs_fat.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <string.h>
#include <errno.h>
#include <sys/stat.h>

static const char *TAG = "ext_flash";

static esp_flash_t *s_ext_flash = NULL;
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;
static const esp_partition_t *s_fat_partition = NULL;

// --- Low-level SPI helpers used by the rescue probe ---------------------------

static spi_device_handle_t s_probe_dev = NULL;

static esp_err_t probe_open(int mode, int freq_hz)
{
    spi_device_interface_config_t cfg = {
        .clock_speed_hz = freq_hz,
        .mode = mode,
        .spics_io_num = EXT_FLASH_CS,
        .queue_size = 1,
        .flags = 0,
    };
    return spi_bus_add_device(SPI2_HOST, &cfg, &s_probe_dev);
}

static void probe_close(void)
{
    if (s_probe_dev) {
        spi_bus_remove_device(s_probe_dev);
        s_probe_dev = NULL;
    }
}

// Send a 1-byte command, optionally read N bytes back into rx (rx may be NULL).
static esp_err_t probe_cmd(uint8_t cmd, uint8_t *rx, size_t rx_len)
{
    uint8_t tx_buf[8] = {cmd, 0, 0, 0, 0, 0, 0, 0};
    uint8_t rx_buf[8] = {0};
    size_t total = 1 + rx_len;
    if (total > sizeof(tx_buf)) return ESP_ERR_INVALID_SIZE;
    spi_transaction_t t = {
        .length = total * 8,
        .rxlength = total * 8,
        .tx_buffer = tx_buf,
        .rx_buffer = rx_buf,
    };
    esp_err_t ret = spi_device_polling_transmit(s_probe_dev, &t);
    if (ret == ESP_OK && rx) memcpy(rx, &rx_buf[1], rx_len);
    return ret;
}

// Send command + 24-bit address + read N bytes (for 0x90 Read Mfr/Dev ID).
static esp_err_t probe_cmd_addr(uint8_t cmd, uint32_t addr, uint8_t *rx, size_t rx_len)
{
    uint8_t tx_buf[8] = {cmd, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF, 0, 0, 0, 0};
    uint8_t rx_buf[8] = {0};
    size_t total = 4 + rx_len;
    if (total > sizeof(tx_buf)) return ESP_ERR_INVALID_SIZE;
    spi_transaction_t t = {
        .length = total * 8,
        .rxlength = total * 8,
        .tx_buffer = tx_buf,
        .rx_buffer = rx_buf,
    };
    esp_err_t ret = spi_device_polling_transmit(s_probe_dev, &t);
    if (ret == ESP_OK && rx) memcpy(rx, &rx_buf[4], rx_len);
    return ret;
}

static bool valid_id(const uint8_t *b, size_t n)
{
    bool all_ff = true, all_00 = true;
    for (size_t i = 0; i < n; i++) {
        if (b[i] != 0xFF) all_ff = false;
        if (b[i] != 0x00) all_00 = false;
    }
    return !all_ff && !all_00;
}

// Comprehensive rescue probe. Tries to recover the chip from any stuck state
// and then read its ID via two different commands in two SPI modes.
// Returns ESP_OK as soon as ANY method returns a non-trivial ID.
static esp_err_t probe_chip_with_rescue(void)
{
    const int freqs_hz[] = {1000000};         // 1 MHz only — slow and safe
    const int modes[] = {0, 3};               // W25Q128JV supports both

    for (size_t mi = 0; mi < sizeof(modes)/sizeof(modes[0]); mi++) {
        int mode = modes[mi];
        for (size_t fi = 0; fi < sizeof(freqs_hz)/sizeof(freqs_hz[0]); fi++) {
            int freq = freqs_hz[fi];
            ESP_LOGW(TAG, "--- Rescue attempt: SPI mode %d @ %d Hz ---", mode, freq);
            if (probe_open(mode, freq) != ESP_OK) {
                ESP_LOGE(TAG, "  probe_open failed");
                continue;
            }

            // Step 1: Exit Continuous Read Mode — clock out a byte of 0xFF
            // with no command interpretation. Done by sending 0xFF as "cmd"
            // (which W25Q128JV treats as no-op if it's in continuous read mode).
            ESP_LOGW(TAG, "  step 1: exit continuous read mode (0xFF)");
            probe_cmd(0xFF, NULL, 0);
            vTaskDelay(pdMS_TO_TICKS(2));

            // Step 2: Software Reset Enable + Reset (0x66, 0x99) — separate
            // CS pulses required. Resets chip to default standby.
            ESP_LOGW(TAG, "  step 2: software reset (0x66 + 0x99)");
            probe_cmd(0x66, NULL, 0);
            vTaskDelay(pdMS_TO_TICKS(1));
            probe_cmd(0x99, NULL, 0);
            vTaskDelay(pdMS_TO_TICKS(2));  // tRST = 30us for W25Q128JV

            // Step 3: Release from Power-Down (0xAB). W25Q128JV may sit in
            // deep power-down after a partial init from a previous boot.
            ESP_LOGW(TAG, "  step 3: release power-down (0xAB)");
            probe_cmd(0xAB, NULL, 0);
            vTaskDelay(pdMS_TO_TICKS(2));  // tRES1 = 20us max

            // Step 4a: Try JEDEC ID (0x9F) — returns 3 bytes
            uint8_t jedec[3] = {0};
            if (probe_cmd(0x9F, jedec, 3) == ESP_OK) {
                ESP_LOGW(TAG, "  JEDEC ID (0x9F): %02X %02X %02X  (W25Q128JV = EF 40 18)",
                         jedec[0], jedec[1], jedec[2]);
                if (valid_id(jedec, 3)) {
                    ESP_LOGI(TAG, "  *** CHIP RESPONDED to JEDEC ID — alive! ***");
                    probe_close();
                    return ESP_OK;
                }
            }

            // Step 4b: Try Read Manufacturer/Device ID (0x90 + addr 0x000000)
            // returns 2 bytes: mfr_id, device_id  (EF, 17 for W25Q128JV)
            uint8_t mfd[2] = {0};
            if (probe_cmd_addr(0x90, 0x000000, mfd, 2) == ESP_OK) {
                ESP_LOGW(TAG, "  Read Mfr/Dev (0x90): %02X %02X  (W25Q128JV = EF 17)",
                         mfd[0], mfd[1]);
                if (valid_id(mfd, 2)) {
                    ESP_LOGI(TAG, "  *** CHIP RESPONDED to 0x90 — alive! ***");
                    probe_close();
                    return ESP_OK;
                }
            }

            probe_close();
        }
    }
    ESP_LOGE(TAG, "All rescue attempts failed across modes 0 and 3.");
    return ESP_FAIL;
}

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

    // Bringup rescue: try to wake/reset the chip and read its ID via two
    // commands in two SPI modes. If any of them succeeds the chip is alive.
    if (probe_chip_with_rescue() != ESP_OK) {
        ESP_LOGE(TAG, "Rescue probe could not get any response from the chip.");
        // Continue anyway so esp_flash_init produces its own diagnostic.
    }

    // Configure external flash device.
    // Start conservative: 5 MHz + standard fast-read. Once link is proven on
    // this PCB, frequency can be bumped (40 MHz) and io_mode set to DIO.
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
