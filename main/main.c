#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "state.h"
#include "peripherals/rc.h"
#include "connectivity/spi.h"
#include "control/data_logger.h"
#include "utils.h"

_Noreturn void process_second(void *args) {
    State *state = args;
    spi_init(state);
    data_logger_init(state);

    while (1) {
        // Process data
        data_logger_process(state);
    }
    vTaskDelete(NULL);
}

_Noreturn void process_main(State *state) {
    rc_init();

    while (1) {
        // Collect data
        rc_collect_data(state);
    }
}

// Running on main core
void app_main(void) {
    State state = {0};

    // Init second core
    portBASE_TYPE result = xTaskCreatePinnedToCore(&process_second, "process_second",
                                                   3584 + 512, &state,
                                                   0, NULL, 1);
    if (result != pdTRUE) {
        printf("Failed to create task for second core\n");
    }

    process_main(&state);
}
