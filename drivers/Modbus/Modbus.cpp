#include "Modbus.h"
#include "esp_log.h"

static const char *TAG = "ModbusRTU";

// Enumeration of all supported CIDs for device
enum {
    CID_DEV_NAME1 = 0,
    CID_DEV_STATUS1,
    CID_HUMIDITY_DATA_1,
    CID_TEMP_DATA_1,
    CID_DEV_NAME2,
    CID_DEV_STATUS2,
    CID_HUMIDITY_DATA_2,
    CID_TEMP_DATA_2,
    CID_DEV_NAME3,
    CID_DEV_STATUS3,
    CID_HUMIDITY_DATA_3,
    CID_TEMP_DATA_3
};

/**
 * @brief Modbus function codes (commands) for master requests.
 */
enum {
    MB_FUNC_READ_COILS = 0x01,                  // Read Coils (0x01)
    MB_FUNC_READ_DISCRETE_INPUTS = 0x02,        // Read Discrete Inputs (0x02)
    MB_FUNC_READ_HOLDING_REGISTER = 0x03,       // Read Holding Registers (0x03)
    MB_FUNC_READ_INPUT_REGISTER = 0x04,         // Read Input Registers (0x04)
    MB_FUNC_WRITE_SINGLE_COIL = 0x05,           // Write Single Coil (0x05)
    MB_FUNC_WRITE_SINGLE_REGISTER = 0x06,       // Write Single Register (0x06)
    MB_FUNC_WRITE_MULTIPLE_COILS = 0x0F,        // Write Multiple Coils (0x0F)
    MB_FUNC_WRITE_MULTIPLE_REGISTERS = 0x10,    // Write Multiple Registers (0x10)
    MB_FUNC_READWRITE_MULTIPLE_REGISTERS = 0x17 // Read/Write Multiple Registers (0x17)
};

enum {
    MB_DEVICE_ADDR1 = 1,
    MB_DEVICE_ADDR2,
    MB_DEVICE_ADDR3,
};

// Modbus parameter descriptors for three devices
mb_parameter_descriptor_t device_parameters[] = {
    // Device 1
    { CID_DEV_NAME1, STR("Device name 1"), STR("__"), MB_DEVICE_ADDR1, MB_PARAM_HOLDING, 0, 8,
                    0, PARAM_TYPE_ASCII, PARAM_SIZE_ASCII, OPTS( 0, 0, 0 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_DEV_STATUS1, STR("Device status 1"), STR("--"), MB_DEVICE_ADDR1, MB_PARAM_HOLDING, 8, 1,
                    0, PARAM_TYPE_U16, PARAM_SIZE_U16, OPTS( 0, 0, 0 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_HUMIDITY_DATA_1, STR("Humidity 1"), STR("%"), MB_DEVICE_ADDR1, MB_PARAM_HOLDING, 9, 2,
                    0, PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, OPTS( 0, 100, 1 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_TEMP_DATA_1, STR("Temperature 1"), STR("C"), MB_DEVICE_ADDR1, MB_PARAM_HOLDING, 11, 2,
                    0, PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, OPTS( -20, 50, 1 ), PAR_PERMS_READ_WRITE_TRIGGER },

    // Device 2
    { CID_DEV_NAME2, STR("Device name 2"), STR("__"), MB_DEVICE_ADDR2, MB_PARAM_HOLDING, 0, 8,
                    0, PARAM_TYPE_ASCII, PARAM_SIZE_ASCII, OPTS( 0, 0, 0 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_DEV_STATUS2, STR("Device status 2"), STR("--"), MB_DEVICE_ADDR2, MB_PARAM_HOLDING, 8, 1,
                    0, PARAM_TYPE_U16, PARAM_SIZE_U16, OPTS( 0, 0, 0 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_HUMIDITY_DATA_2, STR("Humidity 2"), STR("%"), MB_DEVICE_ADDR2, MB_PARAM_HOLDING, 9, 2,
                    0, PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, OPTS( 0, 100, 1 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_TEMP_DATA_2, STR("Temperature 2"), STR("C"), MB_DEVICE_ADDR2, MB_PARAM_HOLDING, 11, 2,
                    0, PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, OPTS( -20, 50, 1 ), PAR_PERMS_READ_WRITE_TRIGGER },

    // Device 3
    { CID_DEV_NAME3, STR("Device name 3"), STR("__"), MB_DEVICE_ADDR3, MB_PARAM_HOLDING, 0, 8,
                    0, PARAM_TYPE_ASCII, PARAM_SIZE_ASCII, OPTS( 0, 0, 0 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_DEV_STATUS3, STR("Device status 3"), STR("--"), MB_DEVICE_ADDR3, MB_PARAM_HOLDING, 8, 1,
                    0, PARAM_TYPE_U16, PARAM_SIZE_U16, OPTS( 0, 0, 0 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_HUMIDITY_DATA_3, STR("Humidity 3"), STR("%"), MB_DEVICE_ADDR3, MB_PARAM_HOLDING, 9, 2,
                    0, PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, OPTS( 0, 100, 1 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_TEMP_DATA_3, STR("Temperature 3"), STR("C"), MB_DEVICE_ADDR3, MB_PARAM_HOLDING, 11, 2,
                    0, PARAM_TYPE_FLOAT, PARAM_SIZE_FLOAT, OPTS( -20, 50, 1 ), PAR_PERMS_READ_WRITE_TRIGGER },
};

// Calculate number of parameters in the table
uint16_t num_device_parameters = (sizeof(device_parameters) / sizeof(device_parameters[0]));

// The function to get pointer to parameter storage (instance) according to parameter description table
void* ModbusRTU::masterGetParamData(const mb_parameter_descriptor_t* param_descriptor) {
    assert(param_descriptor != NULL);
    void* instance_ptr = NULL;
    if (param_descriptor->param_offset != 0) {
        switch (param_descriptor->mb_param_type) {
            case MB_PARAM_HOLDING:
                instance_ptr = ((void*)&holding_reg_params + param_descriptor->param_offset - 1);
                break;
            case MB_PARAM_INPUT:
                instance_ptr = ((void*)&input_reg_params + param_descriptor->param_offset - 1);
                break;
            case MB_PARAM_COIL:
                instance_ptr = ((void*)&coil_reg_params + param_descriptor->param_offset - 1);
                break;
            case MB_PARAM_DISCRETE:
                instance_ptr = ((void*)&discrete_reg_params + param_descriptor->param_offset - 1);
                break;
            default:
                instance_ptr = NULL;
                break;
        }
    } else {
        ESP_LOGE(TAG, "Wrong parameter offset for CID #%u", (unsigned)param_descriptor->cid);
        assert(instance_ptr != NULL);
    }
    return instance_ptr;
}

ModbusRTU::ModbusRTU(uint8_t slave_id, uart_port_t uart_port, uint32_t baudrate, uart_parity_t parity, mb_mode_type_t mode, int tx_pin, int rx_pin, int rts_pin) {
    this->slave_id = slave_id;
    this->uart_port = uart_port;
    this->tx_pin = tx_pin;
    this->rx_pin = rx_pin;
    this->rts_pin = rts_pin;

    // Set up modbus parameters
    this->comm_info.port = uart_port;
    this->comm_info.mode = mode;
    this->comm_info.baudrate = baudrate;
    this->comm_info.parity = parity;
}

ModbusRTU::~ModbusRTU() {
    if (master_handler != nullptr) {
        mbc_master_destroy();
        master_handler = nullptr;
    }
}

bool ModbusRTU::init() {
    esp_err_t err = ESP_OK;

    // Initialize Modbus controller
    err = mbc_master_init(MB_PORT_SERIAL_MASTER, &master_handler);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Modbus controller: %s", esp_err_to_name(err));
        return false;
    }

    // Configure Modbus communication parameters
    err = mbc_master_setup((void*)&comm_info);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to setup Modbus communication: %s", esp_err_to_name(err));
        return false;
    }

    // Set UART pins
    err = uart_set_pin(uart_port, tx_pin, rx_pin, rts_pin, UART_PIN_NO_CHANGE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set UART pins: %s", esp_err_to_name(err));
        return false;
    }

    // Start Modbus controller
    err = mbc_master_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start Modbus controller: %s", esp_err_to_name(err));
        return false;
    }

    // Set parameter descriptor table
    err = mbc_master_set_descriptor(device_parameters, num_device_parameters);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set parameter descriptor table: %s", esp_err_to_name(err));
        return false;
    }

    ESP_LOGI(TAG, "Modbus RTU initialized successfully");
    return true;
}

bool ModbusRTU::readHoldingRegisters(uint16_t address, uint16_t quantity, uint16_t* response) {
    mb_param_request_t request = {
        .slave_addr = slave_id,
        .command = MB_FUNC_READ_HOLDING_REGISTER,
        .reg_start = address,
        .reg_size = quantity
    };

    esp_err_t err = mbc_master_send_request(&request, response);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read holding registers: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

bool ModbusRTU::writeSingleRegister(uint16_t address, uint16_t value) {
    mb_param_request_t request = {
        .slave_addr = slave_id,
        .command = MB_FUNC_WRITE_SINGLE_REGISTER,
        .reg_start = address,
        .reg_size = 1
    };

    esp_err_t err = mbc_master_send_request(&request, &value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write single register: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

bool ModbusRTU::writeMultipleRegisters(uint16_t address, uint16_t quantity, uint16_t* values) {
    mb_param_request_t request = {
        .slave_addr = slave_id,
        .command = MB_FUNC_WRITE_MULTIPLE_REGISTERS,
        .reg_start = address,
        .reg_size = quantity
    };

    esp_err_t err = mbc_master_send_request(&request, values);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write multiple registers: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

bool ModbusRTU::readCoils(uint16_t address, uint16_t quantity, uint8_t* response) {
    mb_param_request_t request = {
        .slave_addr = slave_id,
        .command = MB_FUNC_READ_COILS,
        .reg_start = address,
        .reg_size = quantity
    };

    esp_err_t err = mbc_master_send_request(&request, response);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read coils: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

bool ModbusRTU::writeSingleCoil(uint16_t address, bool value) {
    uint8_t coil_value = value ? 0xFF : 0x00;
    mb_param_request_t request = {
        .slave_addr = slave_id,
        .command = MB_FUNC_WRITE_SINGLE_COIL,
        .reg_start = address,
        .reg_size = 1
    };

    esp_err_t err = mbc_master_send_request(&request, &coil_value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write single coil: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

bool ModbusRTU::writeMultipleCoils(uint16_t address, uint16_t quantity, uint8_t* values) {
    mb_param_request_t request = {
        .slave_addr = slave_id,
        .command = MB_FUNC_WRITE_MULTIPLE_COILS,
        .reg_start = address,
        .reg_size = quantity
    };

    esp_err_t err = mbc_master_send_request(&request, values);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write multiple coils: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}