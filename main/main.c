#include <sys/cdefs.h>
#include <stdio.h>
#include <driver/adc.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "state.h"
#include "utils.h"
#include "control/control.h"
#include "connectivity/i2c.h"
#include "connectivity/spi.h"
#include "peripherals/display/display.h"
#include "peripherals/gpsgsm.h"
#include "control/data_logger.h"
#include "control/security.h"

#if WIFI_ENABLE
#include "connectivity/wifi.h"
#endif
#if BLUETOOTH_ENABLE
#include "connectivity/bluetooth.h"
#endif


_Noreturn void process_gui(void *args) {
    State *state = args;
    display_init();

    while (1) {
        display_update(state);
    }
    vTaskDelete(NULL);
}

_Noreturn void process_main(State *state) {
    adc1_config_width(ADC_RESOLUTION - 9);
    i2c_init();
    spi_init(state);
    security_init();
    control_init(state);
    data_logger_init(state);
    gpsgsm_init();
#if BLUETOOTH_ENABLE
    bluetooth_init(state);
#endif
#if WIFI_ENABLE
    wifi_init(state);
#endif

    state->is_booting = false;

    while (1) {
        // Collect data
        control_read_can_bus(state);
        control_read_analog_sensors(state);
        control_read_user_input(state);
        gpsgsm_process(state);

        // Process data
        security_step(state);
        control_mpu_power(state);
        control_door_lock(state);
        control_trip_logger(state);
        control_cruise_control(state);
        control_crash_detection(state);

        data_logger_process(state);

#if WIFI_ENABLE
        wifi_scan(state);
#endif
    }
}

void init() {
    nvs_init();
}

// Running on main core
void app_main(void) {
    State state = {
            .is_booting = true,
            .power_off_count_down_sec = -1,
            .location.time.timezone = 2,    // GMT+2
    };

    init();

    // Init second core
    portBASE_TYPE result = xTaskCreatePinnedToCore(&process_gui, "process_gui",
                                                   3584 + 512, &state,
                                                   0, NULL, 1);
    if (result != pdTRUE) {
        printf("Failed to create task for second core\n");
    }

    process_main(&state);
}
