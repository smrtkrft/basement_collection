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

// --- Bit-bang diagnostic: bypass SPI peripheral entirely --------------------
// Manually drive CS/CLK/MOSI and sample MISO via gpio_get_level().
// Used as the ultimate diagnostic when SPI master + rescue probe both fail —
// proves whether MISO can carry a chip-driven signal back to the ESP at all.

static inline void bb_delay(void)
{
    // ~10us half-cycle → ~50 kHz SPI clock. Extremely conservative.
    for (volatile int i = 0; i < 400; i++) { __asm__ __volatile__("nop"); }
}

static uint8_t bb_xfer_byte(uint8_t tx)
{
    uint8_t rx = 0;
    for (int b = 7; b >= 0; b--) {
        // Drive MOSI while CLK is low (SPI mode 0)
        gpio_set_level(EXT_FLASH_MOSI, (tx >> b) & 1);
        bb_delay();
        // Rising edge: chip samples MOSI, chip drives MISO -> we sample MISO
        gpio_set_level(EXT_FLASH_CLK, 1);
        bb_delay();
        if (gpio_get_level(EXT_FLASH_MISO)) {
            rx |= (1 << b);
        }
        // Falling edge
        gpio_set_level(EXT_FLASH_CLK, 0);
    }
    return rx;
}

static void bitbang_jedec_probe(void)
{
    ESP_LOGW(TAG, "===== BIT-BANG JEDEC PROBE (bypasses SPI peripheral) =====");

    // Configure all 4 pins as plain GPIOs. MISO with internal pull-up so we
    // can distinguish "chip driving LOW" from "MISO floating".
    gpio_config_t out_cfg = {
        .pin_bit_mask = (1ULL << EXT_FLASH_CS) | (1ULL << EXT_FLASH_CLK) |
                        (1ULL << EXT_FLASH_MOSI),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&out_cfg);
    gpio_config_t in_cfg = {
        .pin_bit_mask = 1ULL << EXT_FLASH_MISO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&in_cfg);

    // Idle state
    gpio_set_level(EXT_FLASH_CS, 1);
    gpio_set_level(EXT_FLASH_CLK, 0);
    gpio_set_level(EXT_FLASH_MOSI, 0);
    vTaskDelay(pdMS_TO_TICKS(10));

    int miso_idle = gpio_get_level(EXT_FLASH_MISO);
    ESP_LOGW(TAG, "  MISO idle (CS high, pull-up enabled) = %d  %s",
             miso_idle, miso_idle ? "(floating HIGH = normal)" : "(LOW = pulled down somewhere)");

    // Transaction: assert CS, send 0x9F, read 3 bytes
    gpio_set_level(EXT_FLASH_CS, 0);
    bb_delay();

    uint8_t discard = bb_xfer_byte(0x9F);  // cmd byte; rx side is junk
    uint8_t b0 = bb_xfer_byte(0x00);
    uint8_t b1 = bb_xfer_byte(0x00);
    uint8_t b2 = bb_xfer_byte(0x00);

    bb_delay();
    gpio_set_level(EXT_FLASH_CS, 1);

    int miso_after = gpio_get_level(EXT_FLASH_MISO);

    ESP_LOGW(TAG, "  Bit-bang JEDEC: cmd_echo=%02X  id=%02X %02X %02X  miso_after=%d",
             discard, b0, b1, b2, miso_after);

    if (b0 == 0x00 && b1 == 0x00 && b2 == 0x00) {
        ESP_LOGE(TAG, "  *** Bit-bang also returns 00 00 00 — chip-to-ESP path is broken ***");
        ESP_LOGE(TAG, "  This is conclusive: MISO line cannot carry a chip-driven signal back.");
    } else if (b0 == 0xFF && b1 == 0xFF && b2 == 0xFF) {
        ESP_LOGE(TAG, "  *** Bit-bang returns FF FF FF — MISO floating, chip silent ***");
        ESP_LOGE(TAG, "  Chip is not responding to commands (CS/CLK/MOSI reach but no DO output).");
    } else {
        ESP_LOGI(TAG, "  *** CHIP RESPONDED via bit-bang — alive! ***");
        ESP_LOGI(TAG, "  (SPI driver layer must have been the problem.)");
    }
    ESP_LOGW(TAG, "===== END BIT-BANG PROBE =====");
}

// --- Low-level SPI helpers used by the rescue probe ---------------------------

static spi_device_handle_t s_probe_dev = NULL;

static esp_err_t probe_open(int mode, int freq_hz)
{
    // cs_ena_pretrans/posttrans add ~2 SPI clock cycles of CS-low margin around
    // each transaction. Defensive against tight setup/hold windows. Harmless on
    // a standard chip; helpful if the link is marginal.
    spi_device_interface_config_t cfg = {
        .clock_speed_hz = freq_hz,
        .mode = mode,
        .spics_io_num = EXT_FLASH_CS,
        .queue_size = 1,
        .flags = 0,
        .cs_ena_pretrans = 2,
        .cs_ena_posttrans = 2,
    };
    return spi_bus_add_device(SPI2_HOST, &cfg, &s_probe_dev);
}

// Send raw bytes from a buffer (no automatic command framing). Used to flush
// stuck QPI state by clocking out a string of 0xFF before the real reset cmd.
static esp_err_t probe_raw(const uint8_t *tx, size_t len)
{
    spi_transaction_t t = {
        .length = len * 8,
        .rxlength = 0,
        .tx_buffer = tx,
        .rx_buffer = NULL,
    };
    return spi_device_polling_transmit(s_probe_dev, &t);
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

            // Step 0: Flush any partial QPI nibble state. In QPI mode the chip
            // treats each clocked byte as 2 nibbles; clocking 8 bytes of 0xFF
            // with no command resets the chip's QPI byte boundary.
            ESP_LOGW(TAG, "  step 0: flush QPI nibble state (8x 0xFF)");
            uint8_t flush[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            probe_raw(flush, sizeof(flush));
            vTaskDelay(pdMS_TO_TICKS(1));

            // Step 1: Reset QPI mode (0xF5). If the chip is in QPI/QSPI mode
            // it ignores the standard SPI reset (0x66+0x99). 0xF5 is the
            // Macronix RSTQIO command that drops it back to standard SPI.
            // Harmless if chip is already in standard SPI mode.
            ESP_LOGW(TAG, "  step 1: reset QPI mode (0xF5 RSTQIO)");
            probe_cmd(0xF5, NULL, 0);
            vTaskDelay(pdMS_TO_TICKS(2));

            // Step 2: Exit Continuous Read Mode (clock out 0xFF as cmd).
            ESP_LOGW(TAG, "  step 2: exit continuous read mode (0xFF)");
            probe_cmd(0xFF, NULL, 0);
            vTaskDelay(pdMS_TO_TICKS(2));

            // Step 3: Software Reset Enable + Reset (0x66, 0x99). Separate
            // CS pulses required. Resets chip to default standby.
            ESP_LOGW(TAG, "  step 3: software reset (0x66 + 0x99)");
            probe_cmd(0x66, NULL, 0);
            vTaskDelay(pdMS_TO_TICKS(1));
            probe_cmd(0x99, NULL, 0);
            vTaskDelay(pdMS_TO_TICKS(2));  // tRST ~30us; we give 2ms

            // Step 4: Release from Power-Down (0xAB). Chip may sit in deep
            // power-down after a partial init from a previous boot.
            ESP_LOGW(TAG, "  step 4: release power-down (0xAB)");
            probe_cmd(0xAB, NULL, 0);
            vTaskDelay(pdMS_TO_TICKS(2));  // tRES1 ~20us; we give 2ms

            // Step 4a: Try JEDEC ID (0x9F) — returns 3 bytes:
            //   manufacturer_id, memory_type, capacity
            //   MX25L25645G       = C2 20 19    (Macronix, 256Mbit)
            //   W25Q128JV         = EF 40 18    (Winbond,  128Mbit)
            uint8_t jedec[3] = {0};
            if (probe_cmd(0x9F, jedec, 3) == ESP_OK) {
                ESP_LOGW(TAG, "  JEDEC ID (0x9F): %02X %02X %02X  (MX25L25645G = C2 20 19)",
                         jedec[0], jedec[1], jedec[2]);
                if (valid_id(jedec, 3)) {
                    ESP_LOGI(TAG, "  *** CHIP RESPONDED to JEDEC ID — alive! ***");
                    probe_close();
                    return ESP_OK;
                }
            }

            // Step 4b: Try Read Manufacturer/Device ID (0x90 + addr 0x000000)
            // returns 2 bytes: mfr_id, device_id
            //   MX25L25645G = C2 19
            //   W25Q128JV   = EF 17
            uint8_t mfd[2] = {0};
            if (probe_cmd_addr(0x90, 0x000000, mfd, 2) == ESP_OK) {
                ESP_LOGW(TAG, "  Read Mfr/Dev (0x90): %02X %02X  (MX25L25645G = C2 19)",
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

    // Diagnostic: bit-bang JEDEC probe BEFORE binding the SPI peripheral.
    // If the chip responds here but not via the SPI driver, the driver is the
    // problem. If it doesn't respond here either, the chip-to-ESP signal path
    // is the problem (regardless of what the multimeter tests showed).
    bitbang_jedec_probe();

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

    // Power-up settle: Macronix tVSL spec is ~800us but some boards need
    // tens of ms for Vcc to fully stabilize before the chip will accept
    // the first command. Cheap insurance.
    vTaskDelay(pdMS_TO_TICKS(50));

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
