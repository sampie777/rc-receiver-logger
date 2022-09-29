//
// Created by samuel on 29-9-22.
//
#ifndef RC_RECEIVER_LOGGER_RC_H
#define RC_RECEIVER_LOGGER_RC_H

#include "../state.h"

void rc_init();

void rc_collect_data(State *state);

#endif //RC_RECEIVER_LOGGER_RC_H