/**
 * @file ds3231.h
 * @brief Real-time clock driver for the DS3231
 */
#ifndef MAIN_DS3231_H_
#define MAIN_DS3231_H_

#include <time.h>
#include <stdbool.h>
#include "driver/i2c.h"

#include "I2CMaster.h"

#define DS3231_ADDR 0x68 //!< I2C address

#define DS3231_STAT_OSCILLATOR 0x80
#define DS3231_STAT_32KHZ      0x08
#define DS3231_STAT_BUSY       0x04
#define DS3231_STAT_ALARM_2    0x02
#define DS3231_STAT_ALARM_1    0x01

#define DS3231_CTRL_OSCILLATOR    0x80
#define DS3231_CTRL_SQUAREWAVE_BB 0x40
#define DS3231_CTRL_TEMPCONV      0x20
#define DS3231_CTRL_ALARM_INTS    0x04
#define DS3231_CTRL_ALARM2_INT    0x02
#define DS3231_CTRL_ALARM1_INT    0x01

#define DS3231_ALARM_WDAY   0x40
#define DS3231_ALARM_NOTSET 0x80

#define DS3231_ADDR_TIME    0x00
#define DS3231_ADDR_ALARM1  0x07
#define DS3231_ADDR_ALARM2  0x0b
#define DS3231_ADDR_CONTROL 0x0e
#define DS3231_ADDR_STATUS  0x0f
#define DS3231_ADDR_AGING   0x10
#define DS3231_ADDR_TEMP    0x11

#define DS3231_12HOUR_FLAG  0x40
#define DS3231_12HOUR_MASK  0x1f
#define DS3231_PM_FLAG      0x20
#define DS3231_MONTH_MASK   0x1f

class DS3231 {
    public:
        // Constructor
        DS3231(I2CMaster* i2c_master, uint8_t address = 0x68);
    
        // Initialize the DS3231
        esp_err_t init();
    
        // Set the time on the DS3231
        esp_err_t setTime(struct tm* time);
    
        // Get the time from the DS3231
        esp_err_t getTime(struct tm* time);
    
        // Get the raw temperature value
        esp_err_t getRawTemperature(int16_t* temp);
    
        // Get the temperature as an integer
        esp_err_t getTemperatureInteger(int8_t* temp);
    
        // Get the temperature as a float
        esp_err_t getTemperatureFloat(float* temp);
    
    private:
        // Helper functions
        uint8_t bcd2dec(uint8_t val);
        uint8_t dec2bcd(uint8_t val);
    
        // I2C master instance
        I2CMaster* i2c_master;
    
        // DS3231 I2C address
        uint8_t address;
    };
    
#endif /* MAIN_DS3231_H_ */
