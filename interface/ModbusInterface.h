#pragma once

#include <cstdint>
#include <cstddef>

class ModbusInterface {
    public:
        virtual bool readHoldingRegisters(uint16_t address, uint16_t quantity, uint16_t* response) = 0;
        virtual bool writeSingleRegister(uint16_t address, uint16_t value) = 0;
        virtual bool writeMultipleRegisters(uint16_t address, uint16_t quantity, uint16_t* values) = 0;
    
        virtual bool readCoils(uint16_t address, uint16_t quantity, uint8_t* response) = 0;
        virtual bool writeSingleCoil(uint16_t address, bool value) = 0;
        virtual bool writeMultipleCoils(uint16_t address, uint16_t quantity, uint8_t* values) = 0;
    
        virtual ~ModbusInterface() = default;
    };