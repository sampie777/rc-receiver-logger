//
// Created by samuel on 29-9-22.
//

#include <driver/gpio.h>
#include <esp_timer.h>
#include "rc.h"
#include "../utils.h"

void rc_init() {
    gpio_set_direction(RC_CHANNEL_0_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(RC_CHANNEL_1_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(RC_CHANNEL_2_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(RC_CHANNEL_3_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(RC_CHANNEL_4_PIN, GPIO_MODE_INPUT);
}

void track_errors(RcChannel *channel) {
    static int error_count = 0;
    if (channel->prev_cycle_length > 8000) {
        error_count++;
    } else {
        error_count--;
        if (error_count < 0)
            error_count = 0;
    }
    if (error_count > 50) {
        utils_reboot();
        error_count = 0;
    }
}

void process_channel(RcChannel *channel, gpio_num_t pin) {
    int64_t current_time = esp_timer_get_time();

    // Check cycle start/end
    if (!gpio_get_level(pin)) {
        channel->cycle_started = false;
    } else if (!channel->cycle_started) {
        channel->prev_value = channel->value;
        channel->prev_cycle_length = current_time - channel->cycle_start_time;
        channel->value = 0;
        channel->cycle_started = true;
        channel->cycle_start_time = current_time;

        track_errors(channel);
    }

    if (channel->value != 0)
        return;

    if (!gpio_get_level(pin))
        channel->value = current_time - channel->cycle_start_time;
}

void rc_collect_data(State *state) {
    process_channel(&(state->rc.channel0), RC_CHANNEL_0_PIN);
    process_channel(&(state->rc.channel1), RC_CHANNEL_1_PIN);
    process_channel(&(state->rc.channel2), RC_CHANNEL_2_PIN);
    process_channel(&(state->rc.channel3), RC_CHANNEL_3_PIN);
    process_channel(&(state->rc.channel4), RC_CHANNEL_4_PIN);
}
