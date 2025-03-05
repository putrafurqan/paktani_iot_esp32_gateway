#include "I2cTool.h"
#include "esp_log.h"

#define I2CDEV_TIMEOUT 1000  // Timeout in milliseconds

I2CTool::I2CTool(i2c_port_t port) : i2c_port(port) {}

esp_err_t I2CTool::read(uint8_t addr, const void* out_data, size_t out_size, void* in_data, size_t in_size) {
    if (!in_data || !in_size) return ESP_ERR_INVALID_ARG;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (out_data && out_size) {
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write(cmd, static_cast<const uint8_t*>(out_data), out_size, true);
    }
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, static_cast<uint8_t*>(in_data), in_size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    esp_err_t res = i2c_master_cmd_begin(i2c_port, cmd, I2CDEV_TIMEOUT / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return res;
}

esp_err_t I2CTool::write(uint8_t addr, const void* out_reg, size_t out_reg_size, const void* out_data, size_t out_size) {
    if (!out_reg || !out_reg_size) return ESP_ERR_INVALID_ARG;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, static_cast<const uint8_t*>(out_reg), out_reg_size, true);

    if (out_data && out_size) {
        i2c_master_write(cmd, static_cast<const uint8_t*>(out_data), out_size, true);
    }

    i2c_master_stop(cmd);

    esp_err_t res = i2c_master_cmd_begin(i2c_port, cmd, I2CDEV_TIMEOUT / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return res;
}
