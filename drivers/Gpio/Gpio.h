/**
 * @file Gpio.h
 * @brief This file defines the Gpio class for ESP32 microcontrollers.
 *
 * This class provides an interface to interact with GPIO pins, allowing for initialization,
 * reading, writing, and toggling pin states. It supports ESP32.
 */
#pragma once

#include "driver/gpio.h"
#include "../../interface/GpioInterface.h"

class Gpio : public GPIOInterface {
public:
    /**
     * @brief Constructor for the Gpio class.
     * @param pin The GPIO pin number.
     * @param mode The GPIO mode (input, output, etc.).
     * @param pull_up_enable Enable or disable the internal pull-up resistor.
     * @param pull_down_enable Enable or disable the internal pull-down resistor.
     */
    Gpio(gpio_num_t pin, gpio_mode_t mode, bool pull_up_enable = false, bool pull_down_enable = false);

    /**
     * @brief Destructor for the Gpio class.
     */
    ~Gpio() override;

    /**
      * @brief Initialize the GPIO pin.
      */
    void init() override;

    /**
     * @brief Write a state to the GPIO pin.
     * @param state The state to write (true = high, false = low).
     */
    void write(bool state) override;

    /**
     * @brief Read the current state of the GPIO pin.
     * @return The current state of the GPIO pin (true = high, false = low).
     */
    bool read() override;

    /**
     * @brief Toggle the current state of the GPIO pin.
     */
    void toggle() override;

    /**
     * @brief attach interrupt
     * 
     */
    void attachInterrupt(gpio_num_t gpio_pin, gpio_isr_t handler);

private:
    gpio_num_t pin_;          // GPIO pin number
    gpio_mode_t mode_;        // GPIO mode
    bool pull_up_enable_;     // Internal pull-up resistor
    bool pull_down_enable_;   // Internal pull-down resistor
    bool state_;              // Current state of the GPIO pin
};