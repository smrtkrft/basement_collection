idf_component_register(
    SRCS "ext_flash.c"
    INCLUDE_DIRS "include"
    REQUIRES driver spi_flash fatfs vfs esp_partition pin_config
)
