#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "ds3231.h"

#define CHECK_ARG(ARG) do { if (!ARG) return ESP_ERR_INVALID_ARG; } while (0)
#include "DS3231.h"
#include "esp_log.h"

// Constructor
DS3231::DS3231(I2CMaster* i2c_master, uint8_t address)
    : i2c_master(i2c_master), address(address) {}

// Initialize the DS3231
esp_err_t DS3231::init() {
    // No specific initialization required for DS3231
    return ESP_OK;
}

// Convert BCD to decimal
uint8_t DS3231::bcd2dec(uint8_t val) {
    return (val >> 4) * 10 + (val & 0x0f);
}

// Convert decimal to BCD
uint8_t DS3231::dec2bcd(uint8_t val) {
    return ((val / 10) << 4) + (val % 10);
}

// Set the time on the DS3231
esp_err_t DS3231::setTime(struct tm* time) {
    if (!time) return ESP_ERR_INVALID_ARG;

    void* out_data = NULL;
    uint8_t data[7];

    // Convert time to DS3231 format
    data[0] = dec2bcd(time->tm_sec);
    data[1] = dec2bcd(time->tm_min);
    data[2] = dec2bcd(time->tm_hour);
    data[3] = dec2bcd(time->tm_wday + 1);  // DS3231 expects 1-7 for weekdays
    data[4] = dec2bcd(time->tm_mday);
    data[5] = dec2bcd(time->tm_mon + 1);   // DS3231 expects 1-12 for months
    data[6] = dec2bcd(time->tm_year - 2000); // DS3231 expects years since 2000

    // Write time data to DS3231
    return i2c_master->write(address, &out_data, 1, data, sizeof(data));
}

// Get the time from the DS3231
esp_err_t DS3231::getTime(struct tm* time) {
    if (!time) return ESP_ERR_INVALID_ARG;

    void* out_data = NULL;
    uint8_t data[7];

    // Read time data from DS3231
    esp_err_t res = i2c_master->read(address, &out_data, 1, data, sizeof(data));
    if (res != ESP_OK) return res;

    // Convert DS3231 format to struct tm
    time->tm_sec = bcd2dec(data[0]);
    time->tm_min = bcd2dec(data[1]);
    if (data[2] & DS3231_12HOUR_FLAG) {
        // 12-hour format
        time->tm_hour = bcd2dec(data[2] & DS3231_12HOUR_MASK) - 1;
        if (data[2] & DS3231_PM_FLAG) time->tm_hour += 12; // PM
    } else {
        // 24-hour format
        time->tm_hour = bcd2dec(data[2]);
    }
    time->tm_wday = bcd2dec(data[3]) - 1; // DS3231 uses 1-7 for weekdays
    time->tm_mday = bcd2dec(data[4]);
    time->tm_mon = bcd2dec(data[5] & DS3231_MONTH_MASK) - 1; // DS3231 uses 1-12 for months
    time->tm_year = bcd2dec(data[6]) + 2000; // DS3231 stores years since 2000
    time->tm_isdst = 0; // No DST information

    return ESP_OK;
}

// Get the raw temperature value
esp_err_t DS3231::getRawTemperature(int16_t* temp) {
    if (!temp) return ESP_ERR_INVALID_ARG;

    uint8_t data[2];

    void* out_data = NULL;

    // Read temperature data from DS3231
    esp_err_t res = i2c_master->read(address, &out_data, 1, data, sizeof(data));
    if (res == ESP_OK) {
        *temp = (int16_t)(int8_t)data[0] << 2 | data[1] >> 6;
    }

    return res;
}

// Get the temperature as an integer
esp_err_t DS3231::getTemperatureInteger(int8_t* temp) {
    if (!temp) return ESP_ERR_INVALID_ARG;

    int16_t raw_temp;
    esp_err_t res = getRawTemperature(&raw_temp);
    if (res == ESP_OK) {
        *temp = raw_temp >> 2;
    }

    return res;
}

// Get the temperature as a float
esp_err_t DS3231::getTemperatureFloat(float* temp) {
    if (!temp) return ESP_ERR_INVALID_ARG;

    int16_t raw_temp;
    esp_err_t res = getRawTemperature(&raw_temp);
    if (res == ESP_OK) {
        *temp = raw_temp * 0.25f;
    }

    return res;
}