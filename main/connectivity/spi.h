//
// Created by samuel on 21-7-22.
//

#ifndef APP_TEMPLATE_SPI_H
#define APP_TEMPLATE_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <driver/spi_master.h>
#include "../state.h"

int spi_init(State *state);

int spi_register_device(spi_device_handle_t *handle, int cs_pin);

#ifdef __cplusplus
}
#endif

#endif //APP_TEMPLATE_SPI_H
