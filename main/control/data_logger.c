//
// Created by samuel on 20-8-22.
//

#include "data_logger.h"
#include "../peripherals/sd_card.h"
#include "../return_codes.h"
#include "../utils.h"
#include "../connectivity/server.h"

void data_logger_upload_all(State *state) {
    static int64_t engine_off_time = 0;
    static uint32_t last_odometer = 0;

    if (state->car.is_ignition_on) {
        engine_off_time = 0;
        return;
    }

    if (engine_off_time == 0) {
        engine_off_time = esp_timer_get_time_ms();
    }

    if (esp_timer_get_time_ms() < engine_off_time + TRIP_LOGGER_ENGINE_OFF_GRACE_TIME_MS) return;

    if (state->car.odometer == last_odometer) return;

    // Can't upload data if WiFi is disconnected
    if (!state->wifi.is_connected) return;

#ifdef DATA_LOGGER_ALL_UPLOAD_URL
    if (server_send_data_log_record(state) != RESULT_OK) {
        // Retry again in X seconds
        engine_off_time = esp_timer_get_time_ms() + TRIP_LOGGER_ENGINE_OFF_GRACE_TIME_MS - TRIP_LOGGER_UPLOAD_RETRY_TIMEOUT_MS;
        return;
    }
#endif

    last_odometer = state->car.odometer;
}

void data_logger_upload_current(State *state) {
    static int64_t last_log_time = 0;

    if (esp_timer_get_time_ms() < last_log_time + DATA_LOGGER_SINGLE_UPLOAD_INTERVAL_MS) return;
    last_log_time = esp_timer_get_time_ms();

    char buffer[512];
    sprintf(buffer, "{"
                    "'car': {"
                    "  'is_connected': %d,"
                    "  'speed': %.3f,"
                    "},"
                    "'location': {"
                    "  'satellites': %d,"
                    "  'latitude': %.5f,"
                    "  'longitude': %.5f,"
                    "  'ground_speed': %.3f,"
                    "  'ground_heading': %.2f,"
                    "  'time': '%d-%02d-%02d'T'%02d:%02d%02d%+d'"
                    "}"
                    "}",
            state->car.is_connected,
            state->car.speed,
            state->location.satellites,
            state->location.latitude,
            state->location.longitude,
            state->location.ground_speed,
            state->location.ground_heading,
            state->location.time.year,
            state->location.time.month,
            state->location.time.day,
            state->location.time.hours,
            state->location.time.minutes,
            state->location.time.seconds,
            state->location.time.timezone);

#ifdef DATA_LOGGER_SINGLE_UPLOAD_URL
    server_send_data(state, DATA_LOGGER_SINGLE_UPLOAD_URL, buffer, false);
#endif
}

/**
 * Collect data and write to SD card
 * @param state
 */
void data_logger_log_current(State *state) {
    static int64_t last_log_time = 0;

    // Don't log if car isn't on and on the move, so SD card can be swapped safely
    if (!state->car.is_ignition_on && state->car.speed <= 1 && state->car.rpm <= 1) return;

    if (esp_timer_get_time_ms() < last_log_time + DATA_LOGGER_LOG_INTERVAL_MS) return;
    last_log_time = esp_timer_get_time_ms();

    char buffer[256];
    sprintf(buffer,
            "%lld;"         // esp_timer_get_time_ms()
            "%d;"           // state->car.is_connected
            "%d;"           // state->car.is_controller_connected
            "%d;"           // state->car.is_braking
            "%d;"           // state->car.is_ignition_on
            "%.3f;"         // state->car.speed
            "%.1f;"         // state->car.rpm
            "%d;"           // state->car.odometer
            "%d;"           // state->car.gas_pedal_connected
            "%.5f;"         // state->car.gas_pedal
            "%d;"           // state->cruise_control.enabled
            "%.3f;"         // state->cruise_control.target_speed
            "%.5f;"         // state->cruise_control.virtual_gas_pedal
            "%.5f;"         // state->cruise_control.control_value
            "\"%s\";"       // state->wifi.ssid
            "%d;"           // state->wifi.ip.addr
            "%d;"           // state->wifi.is_connected
            "%d;"           // state->bluetooth.connected
            "%d;"           // state->motion.connected
            "%.3f;"         // state->motion.accel_x
            "%.3f;"         // state->motion.accel_y
            "%.3f;"         // state->motion.accel_z
            "%.3f;"         // state->motion.gyro_x
            "%.3f;"         // state->motion.gyro_y
            "%.3f;"         // state->motion.gyro_z
            "%.3f;"         // state->motion.compass_x
            "%.3f;"         // state->motion.compass_y
            "%.3f;"         // state->motion.compass_z
            "%.3f;"         // state->motion.temperature
            "%d;"           // state->location.is_gps_on
            "%d;"           // state->location.quality
            "%d;"           // state->location.satellites
            "%d;"           // state->location.is_effective_positioning
            "%.5f;"         // state->location.latitude
            "%.5f;"         // state->location.longitude
            "%.1f;"         // state->location.altitude
            "%.3f;"         // state->location.ground_speed
            "%.2f;"         // state->location.ground_heading
            "%d-%02d-%02d'T'%02d:%02d%02d%+d;"         // state->location.time
            "\n",
            esp_timer_get_time_ms(),
            state->car.is_connected,
            state->car.is_controller_connected,
            state->car.is_braking,
            state->car.is_ignition_on,
            state->car.speed,
            state->car.rpm,
            state->car.odometer,
            state->car.gas_pedal_connected,
            state->car.gas_pedal,
            state->cruise_control.enabled,
            state->cruise_control.target_speed,
            state->cruise_control.virtual_gas_pedal,
            state->cruise_control.control_value,
            state->wifi.ssid,
            state->wifi.ip.addr,
            state->wifi.is_connected,
            state->bluetooth.connected,
            state->motion.connected,
            state->motion.accel_x,
            state->motion.accel_y,
            state->motion.accel_z,
            state->motion.gyro_x,
            state->motion.gyro_y,
            state->motion.gyro_z,
            state->motion.compass_x,
            state->motion.compass_y,
            state->motion.compass_z,
            state->motion.temperature,
            state->location.is_gps_on,
            state->location.quality,
            state->location.satellites,
            state->location.is_effective_positioning,
            state->location.latitude,
            state->location.longitude,
            state->location.altitude,
            state->location.ground_speed,
            state->location.ground_heading,
            state->location.time.year,
            state->location.time.month,
            state->location.time.day,
            state->location.time.hours,
            state->location.time.minutes,
            state->location.time.seconds,
            state->location.time.timezone
    );

    if (sd_card_file_append(state->storage.filename, buffer) == RESULT_OK) {
        state->storage.is_connected = true;
    } else {
        state->storage.is_connected = false;
    }
}

void data_logger_process(State *state) {
#if SD_ENABLE
    static int64_t last_init_time = 0;
    if (state->storage.is_connected == false) {
        if (esp_timer_get_time_ms() > last_init_time + 3000) {
            data_logger_init(state);
            last_init_time = esp_timer_get_time_ms();
        }
    }
#endif

#if SD_ENABLE
    data_logger_log_current(state);
#endif
    data_logger_upload_current(state);
    data_logger_upload_all(state);
}

void data_logger_init(State *state) {
    printf("[DataLogger] Initializing...\n");

#if SD_ENABLE
    if (sd_card_init() != RESULT_OK) {
        state->storage.is_connected = false;
        printf("[DataLogger] Init failed\n");
        return;
    }
    state->storage.is_connected = true;

    if (state->storage.filename[0] == 0x00) {
        if (sd_card_create_file_incremental(DEVICE_NAME, "data", "csv", state->storage.filename) == RESULT_OVERFLOW) {
            display_set_error_message(state, "SD card full");
        }
        printf("[SD] Using file: %s\n", state->storage.filename);

        sd_card_file_append(state->storage.filename, "timestamp;car_is_connected;car_is_controller_connected;car_is_braking;car_is_ignition_on;car_speed;car_rpm;car_odometer;car_gas_pedal_connected;car_gas_pedal;cruise_control_enabled;cruise_control_target_speed;cruise_control_virtual_gas_pedal;cruise_control_control_value;wifi_ssid;wifi_ip.addr;wifi_is_connected;bluetooth_connected;motion_connected;motion_accel_x;motion_accel_y;motion_accel_z;motion_gyro_x;motion_gyro_y;motion_gyro_z;motion_compass_x;motion_compass_y;motion_compass_z;motion_temperature;location_is_gps_on;location_quality;location_satellites;location_is_effective_positioning;location_latitude;location_longitude;location_altitude;location_ground_speed;location_ground_heading;location_datetime;\n");
    }
#else
    state->storage.is_connected = false;
#endif

    printf("[DataLogger] Init done\n");
}