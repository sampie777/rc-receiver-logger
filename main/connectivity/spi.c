//
// Created by samuel on 21-7-22.
//

#include <driver/spi_common.h>
#include <hal/gpio_types.h>
#include "spi.h"
#include "../return_codes.h"
#include "../peripherals/display/display.h"


int spi_init(State *state) {
    spi_bus_config_t config = {
            .mosi_io_num = SPI_MOSI_PIN,
            .miso_io_num = SPI_MISO_PIN,
            .sclk_io_num = SPI_CLK_PIN,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = (8 * 8)
    };
    int error = spi_bus_initialize(SPI_DEFAULT_HOST, &config, SPI_DMA_CH_AUTO);

    if (error != ESP_OK) {
        printf("Return code for spi_bus_initialize: %d\n", error);
        display_set_error_message(state, "SPI failed");
        return RESULT_FAILED;
    }
    return RESULT_OK;
}

int spi_register_device(spi_device_handle_t *handle, int cs_pin) {
    spi_device_interface_config_t device_config = {
            .mode = 0,
            .clock_speed_hz = 2 * (int) 10e6,
            .spics_io_num = cs_pin,
            .queue_size = 1
    };
    int error = spi_bus_add_device(SPI_DEFAULT_HOST, &device_config, handle);

    if (error != ESP_OK) {
        printf("Return code for spi_bus_add_device: %d\n", error);
        return RESULT_FAILED;
    }
    return RESULT_OK;
}