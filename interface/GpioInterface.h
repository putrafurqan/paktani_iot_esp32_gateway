#pragma once

#include <cstdint>

class GPIOInterface {
public:
    virtual ~GPIOInterface() = default;

    virtual void init() = 0;
    virtual void write(bool state) = 0;
    virtual bool read() = 0;
    virtual void toggle() = 0;

};