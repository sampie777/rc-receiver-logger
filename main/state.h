//
// Created by samuel on 17-7-22.
//

#ifndef APP_TEMPLATE_STATE_H
#define APP_TEMPLATE_STATE_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

typedef struct {
    uint32_t prev_value;
    uint32_t value;
    bool cycle_started;
    int64_t cycle_start_time;
    uint32_t prev_cycle_length;
} RcChannel;

typedef struct {
    RcChannel channel1;
    RcChannel channel2;
    RcChannel channel3;
    RcChannel channel4;
    RcChannel channel5;
    RcChannel channel6;
} RcState;

typedef struct {
    uint8_t is_connected;
    char filename[32];
    char *buffer;
} SDState;

typedef struct {
    SDState storage;
    RcState rc;
} State;

#endif //APP_TEMPLATE_STATE_H
