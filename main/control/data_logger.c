//
// Created by samuel on 20-8-22.
//

#include "data_logger.h"
#include "../return_codes.h"
#include "../utils.h"

#if ENABLE_SD
#include "../peripherals/sd_card.h"
#endif


int data_logger_write(State *state) {
    static int64_t last_log_time = 0;

    if (esp_timer_get_time_ms() < last_log_time + DATA_LOGGER_LOG_MAX_INTERVAL_MS) return RESULT_EMPTY;
    last_log_time = esp_timer_get_time_ms();

#if ENABLE_SD
    if (sd_card_file_append(state->storage.filename, state->storage.buffer) == RESULT_OK) {
        state->storage.is_connected = true;
    } else {
        state->storage.is_connected = false;
    }
#endif
    printf("%s", state->storage.buffer);

    if (strlen(state->storage.buffer) > 0) {
        state->storage.buffer[0] = '\0';
    }
    return RESULT_OK;
}

void data_logger_log_current(State *state) {
    static int64_t last_log_time = 0;

    if (esp_timer_get_time_ms() < last_log_time + 10) return;
    last_log_time = esp_timer_get_time_ms();

    char buffer[64];
    sprintf(buffer,
            "%9lld;"            // esp_timer_get_time_ms()
            "%5.1lf;"           // channel 0
            "%5.1lf;"           // channel 1
            "%5.1lf;"           // channel 2
            "%5.1lf;"           // channel 3
            "%5.1lf;"           // channel 4
            "%6u;"              // Duration 0
            "\n",
            esp_timer_get_time_ms(),
            (double) (max(1000, state->rc.channel0.prev_value) - 1000) / 1000 * 100,
            (double) (max(1000, state->rc.channel1.prev_value) - 1000) / 1000 * 100,
            (double) (max(1000, state->rc.channel2.prev_value) - 1000) / 1000 * 100,
            (double) (max(1000, state->rc.channel3.prev_value) - 1000) / 1000 * 100,
            (double) (max(1000, state->rc.channel4.prev_value) - 1000) / 1000 * 100,
            state->rc.channel0.prev_cycle_length
    );

    if (state->storage.buffer == NULL) {
        state->storage.buffer = malloc(strlen(buffer) + 1);
        state->storage.buffer[0] = '\0';
    } else {
        state->storage.buffer = realloc(state->storage.buffer, strlen(state->storage.buffer) + strlen(buffer) + 1);
    }
    strcat(state->storage.buffer, buffer);
}

void data_logger_process(State *state) {
#if ENABLE_SD
    static int64_t last_init_time = 0;
    if (state->storage.is_connected == false) {
        if (esp_timer_get_time_ms() > last_init_time + 3000) {
            data_logger_init(state);
            last_init_time = esp_timer_get_time_ms();
        }
    }
#endif

    data_logger_write(state);
}

void data_logger_init(State *state) {
#if ENABLE_SD
    printf("[DataLogger] Initializing...\n");

    if (sd_card_init() != RESULT_OK) {
        state->storage.is_connected = false;
        printf("[DataLogger] Init failed\n");
        return;
    }
    state->storage.is_connected = true;

    if (state->storage.filename[0] == 0x00) {
        if (sd_card_create_file_incremental(DEVICE_NAME, "data", "csv", state->storage.filename) == RESULT_OVERFLOW) {
            printf("SD card full");
        }
        printf("[SD] Using file: %s\n", state->storage.filename);

        sd_card_file_append(state->storage.filename, "timestamp;channel1;channel2;channel3;channel4;channel5;period0;\n");
    }

    printf("[DataLogger] Init done\n");
#endif
}