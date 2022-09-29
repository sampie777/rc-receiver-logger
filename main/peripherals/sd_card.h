//
// Created by samuel on 18-8-22.
//

#ifndef APP_TEMPLATE_SD_CARD_H
#define APP_TEMPLATE_SD_CARD_H

#include "../state.h"

int sd_card_init();

void sd_card_deinit(State *state);

void sd_card_test();

int sd_card_file_append(const char *file_name, const char *line);

int sd_card_create_file_incremental(const char *directory, const char *base_file_name, const char *base_file_extension, char *file_name_out);

#endif //APP_TEMPLATE_SD_CARD_H
