#include "state.h"
#include "peripherals/rc.h"
#include "connectivity/spi.h"
#include "control/data_logger.h"

_Noreturn void process_main(State *state) {
    spi_init(state);
    data_logger_init(state);
    rc_init();
    state->is_booting = false;

    while (1) {
        // Collect data
        rc_collect_data(state);

        // Process data
        data_logger_process(state);
    }
}

// Running on main core
void app_main(void) {
    State state = {
            .is_booting = true,
    };

    process_main(&state);
}
