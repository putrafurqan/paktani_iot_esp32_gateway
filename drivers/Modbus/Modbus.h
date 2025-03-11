/**
 * @file Modbus.h
 * @brief 
 */
#pragma once

#include "../../interface/ModbusInterface.h"
#include <cstdint>
#include <cstddef>

#include "driver/uart.h"
#include "mbcontroller.h"

#define MB_PORT_NUM     (CONFIG_MB_UART_PORT_NUM)   // Number of UART port used for Modbus connection
#define MB_DEV_SPEED    (CONFIG_MB_UART_BAUD_RATE)  // The communication speed of the UART

// Note: Some pins on target chip cannot be assigned for UART communication.
// See UART documentation for selected board and target to configure pins using Kconfig.

// The number of parameters that intended to be used in the particular control process
#define MASTER_MAX_CIDS num_device_parameters

// Number of reading of parameters from slave
#define MASTER_MAX_RETRY 30

// Timeout to update cid over Modbus
#define UPDATE_CIDS_TIMEOUT_MS          (500)
#define UPDATE_CIDS_TIMEOUT_TICS        (UPDATE_CIDS_TIMEOUT_MS / portTICK_PERIOD_MS)

// Timeout between polls
#define POLL_TIMEOUT_MS                 (1)
#define POLL_TIMEOUT_TICS               (POLL_TIMEOUT_MS / portTICK_PERIOD_MS)

// The macro to get offset for parameter in the appropriate structure
#define HOLD_OFFSET(field) ((uint16_t)(offsetof(holding_reg_params_t, field) + 1))
#define INPUT_OFFSET(field) ((uint16_t)(offsetof(input_reg_params_t, field) + 1))
#define COIL_OFFSET(field) ((uint16_t)(offsetof(coil_reg_params_t, field) + 1))
// Discrete offset macro
#define DISCR_OFFSET(field) ((uint16_t)(offsetof(discrete_reg_params_t, field) + 1))

#define STR(fieldname) ((const char*)( fieldname ))
// Options can be used as bit masks or parameter limits
#define OPTS(min_val, max_val, step_val) { .opt1 = min_val, .opt2 = max_val, .opt3 = step_val }


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


extern mb_parameter_descriptor_t device_parameters[];

class ModbusRTU : public ModbusInterface {
private:
    uint8_t slave_id;
    uart_port_t uart_port;
    mb_communication_info_t comm_info;
    void* master_handler;
    int tx_pin;
    int rx_pin;
    int rts_pin;

    void* masterGetParamData(const mb_parameter_descriptor_t* param_descriptor);
    // esp_err_t initializeUART();
    // esp_err_t initializeMaster();

public:
    ModbusRTU(uint8_t slave_id, 
             uart_port_t uart_port, 
             uint32_t baudrate, 
             uart_parity_t parity, 
             mb_mode_type_t mode,
             int tx_pin,
             int rx_pin,
             int rts_pin);

    ~ModbusRTU();

    bool init();
    
    // Interface implementations
    bool readHoldingRegisters(uint16_t address, uint16_t quantity, uint16_t* response) override;
    bool writeSingleRegister(uint16_t address, uint16_t value) override;
    bool writeMultipleRegisters(uint16_t address, uint16_t quantity, uint16_t* values) override;

    bool readCoils(uint16_t address, uint16_t quantity, uint8_t* response) override;
    bool writeSingleCoil(uint16_t address, bool value) override;
    bool writeMultipleCoils(uint16_t address, uint16_t quantity, uint8_t* values) override;
};