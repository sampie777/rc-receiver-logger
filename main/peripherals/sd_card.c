//
// Created by samuel on 18-8-22.
//

#include <driver/sdspi_host.h>
#include <sdmmc_cmd.h>
#include <esp_vfs_fat.h>
#include <string.h>
#include <sys/stat.h>
//#include <sys/unistd.h>
#include "sd_card.h"
#include "../return_codes.h"
#include "../utils.h"

static const char *TAG = "SD";
#define MOUNT_POINT "/sdcard"
const char mount_point[] = MOUNT_POINT;

sdmmc_host_t host = SDSPI_HOST_DEFAULT();
sdmmc_card_t *card;

/*
void sd_card_test() {
    // Source: https://github.com/espressif/esp-idf/blob/v4.4.1/examples/storage/sd_card/sdspi/main/sd_card_example_main.c

    // First create a file.
    const char *file_hello = MOUNT_POINT"/hello.txt";

    ESP_LOGI(TAG, "Opening file %s", file_hello);
    FILE *f = fopen(file_hello, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    const char *file_foo = MOUNT_POINT"/foo.txt";

    // Check if destination file exists before renaming
    struct stat st;
    if (stat(file_foo, &st) == 0) {
        // Delete it if it exists
        unlink(file_foo);
    }

    // Rename original file
    ESP_LOGI(TAG, "Renaming file %s to %s", file_hello, file_foo);
    if (rename(file_hello, file_foo) != 0) {
        ESP_LOGE(TAG, "Rename failed");
        return;
    }

    // Open renamed file for reading
    ESP_LOGI(TAG, "Reading file %s", file_foo);
    f = fopen(file_foo, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    // Read a line from file
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);

    // Strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);
}
*/

int sd_card_file_append(const char *file_name, const char *line) {
    char path[64];
    sprintf(path, "%s/%s", MOUNT_POINT, file_name);

    FILE *file = fopen(path, "a");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing: %s", path);
        return RESULT_FAILED;
    }
    fputs(line, file);
    fclose(file);
    return RESULT_OK;
}

void sd_card_create_directory(const char *directory, char *created_directory){
    char path[64];
    char directory_safe_name[32];
    directory_safe_name[min(31, strlen(directory))] = '\0';
    strcpy(directory_safe_name, directory);

    string_char_replace(directory_safe_name, ' ', '_');
    string_char_replace(directory_safe_name, '"', '_');
    string_char_replace(directory_safe_name, '\'', '_');
    string_char_replace(directory_safe_name, '\\', '/');
    string_char_replace(directory_safe_name, '#', '_');
    string_char_replace(directory_safe_name, '.', '_');

    sprintf(path, "%s/%s", MOUNT_POINT, directory_safe_name);

    struct stat st;
    if (stat(path, &st) != 0) {
        printf("[SD] Creating directory '%s'\n", path);
        mkdir(path, 0775);
    }

    strcpy(created_directory, directory_safe_name);
}

/**
 * Create a file with given name and extension. If filename already exists, increment new filename with a number until unique.
 * The new filename will be stored in the file_name parameter.
 * When no new filename can be generated (because all options are taken), the output filename will include the word 'overflow'
 * to indicate it is an overflow file. This file will be usable, although not persistent.
  * @param base_file_name
  * @param iteration        Iteration number to start with
  * @param file_name_out    The new file name will be stored in here (size: 32)
  */
int sd_card_create_file_incremental(const char *directory, const char *base_file_name, const char *base_file_extension, char *file_name_out) {
    char path[128];
    char created_directory[32];
    char new_file_name[64];
    struct stat st;

    sd_card_create_directory(directory, created_directory);

    for (uint16_t i = 0; i < 65535; i++) {
        sprintf(new_file_name, "%s/%s-%d.%s", created_directory, base_file_name, i, base_file_extension);
        sprintf(path, "%s/%s", MOUNT_POINT, new_file_name);

        if (stat(path, &st) != 0) {
            memcpy(file_name_out, new_file_name, 32);
            return RESULT_OK;
        }
    }

    sprintf(new_file_name, "%s/%s-overflow.%s", created_directory, base_file_name, base_file_extension);
    memcpy(file_name_out, new_file_name, 32);
    return RESULT_OVERFLOW;
}

void sd_card_deinit(State *state) {
    // All done, unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");
    state->storage.is_connected = false;
}

int sd_card_init() {
    printf("[SD] Initializing...\n");

    host.max_freq_khz = 15000;
    host.slot = SPI_DEFAULT_HOST;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
            .format_if_mount_failed = false,
            .max_files = 5,
            .allocation_unit_size = 16 * 1024
    };

    sdspi_device_config_t slot = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot.gpio_cs = SD_CHIP_SELECT_PIN;
    slot.host_id = host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    esp_err_t ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                          "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                          "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return RESULT_FAILED;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    sdmmc_card_print_info(stdout, card);

    printf("[SD] Init done: \n"
           "\t-- Card IDentification --\n"
           "\tmanufacturer identification number = %d\n"
           "\tOEM/product identification number = %d\n"
           "\tproduct name (MMC v1 has the longest) = %s\n"
           "\tproduct revision = %d\n"
           "\tproduct serial number = %d\n"
           "\tmanufacturing date = %d\n"
           "\t-- Card Specific Data --\n"
           "\tCSD structure format = %d\n"
           "\tMMC version (for CID format) = %d\n"
           "\ttotal number of sectors = %d\n"
           "\tsector size in bytes = %d\n"
           "\tblock length for reads = %d\n"
           "\tCard Command Class for SD = %d\n"
           "\tMax transfer speed = %d\n",
           card->cid.mfg_id,
           card->cid.oem_id,
           card->cid.name,
           card->cid.revision,
           card->cid.serial,
           card->cid.date,
           card->csd.csd_ver,
           card->csd.mmc_ver,
           card->csd.capacity,
           card->csd.sector_size,
           card->csd.read_block_len,
           card->csd.card_command_class,
           card->csd.tr_speed
    );
    printf("[SD] Init done\n");
    return RESULT_OK;
}
