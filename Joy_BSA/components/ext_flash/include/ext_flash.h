#pragma once

#include "esp_err.h"
#include <stddef.h>

#define EXT_FLASH_MOUNT_POINT "/extflash"
#define EXT_FLASH_AUDIO_DIR  EXT_FLASH_MOUNT_POINT "/audio"

/**
 * Initialize SPI bus, probe external flash chip, and mount FAT filesystem.
 */
esp_err_t ext_flash_init(void);

/**
 * Format the external flash FAT filesystem.
 */
esp_err_t ext_flash_format(void);

/**
 * Get free space on external flash in bytes.
 */
size_t ext_flash_get_free_space(void);

/**
 * Get total space on external flash in bytes.
 */
size_t ext_flash_get_total_space(void);

/**
 * Unmount and deinitialize external flash.
 */
void ext_flash_deinit(void);
