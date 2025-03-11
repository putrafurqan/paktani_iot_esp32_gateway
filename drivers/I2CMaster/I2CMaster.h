#ifndef I2C_TOOL_H
#define I2C_TOOL_H

#include "driver/i2c.h"
#include "esp_err.h"
#include <cstddef>  // For size_t
#include <cstdint>  // For uint8_t

class I2CMaster {
public:
    explicit I2CMaster(i2c_port_t port);

    esp_err_t read(uint8_t addr, const void* out_data, size_t out_size, void* in_data, size_t in_size);
    esp_err_t write(uint8_t addr, const void* out_reg, size_t out_reg_size, const void* out_data, size_t out_size);

private:
    i2c_port_t i2c_port;
};

#endif // I2C_TOOL_H
