//
// Created by samuel on 17-7-22.
//

#ifndef APP_TEMPLATE_CONFIG_H
#define APP_TEMPLATE_CONFIG_H

// Unused GPIOs: 22 (GPIO15 will/must be HIGH on device reset: strapping pin)
// GPIO16-17 are usually connected to the SPI flash and PSRAM integrated on the module
// and therefore should not be used for other purposes (https://docs.espressif.com/projects/esp-idf/en/v4.4.1/esp32/api-reference/peripherals/gpio.html)

#define DEVICE_NAME "RClogger"

#define ENABLE_SD true
#define DATA_LOGGER_LOG_MAX_INTERVAL_MS 1000

#define SPI_MOSI_PIN GPIO_NUM_12
#define SPI_MISO_PIN GPIO_NUM_13
#define SPI_CLK_PIN GPIO_NUM_14
#define SPI_DEFAULT_HOST HSPI_HOST

#define SD_CHIP_SELECT_PIN GPIO_NUM_27

#define RC_CHANNEL_0_PIN GPIO_NUM_26
#define RC_CHANNEL_1_PIN GPIO_NUM_25
#define RC_CHANNEL_2_PIN GPIO_NUM_33
#define RC_CHANNEL_3_PIN GPIO_NUM_32
#define RC_CHANNEL_4_PIN GPIO_NUM_35

#define LED_BUILTIN_PIN 2

#endif //APP_TEMPLATE_CONFIG_H
