#include "Gpio.h"

/**
 * @brief Constructor for the Gpio class.
 * @param pin The GPIO pin number.
 * @param mode The GPIO mode (input, output, etc.).
 * @param pull_up_enable Enable or disable the internal pull-up resistor.
 * @param pull_down_enable Enable or disable the internal pull-down resistor.
 */
Gpio::Gpio(gpio_num_t pin, gpio_mode_t mode, bool pull_up_enable, bool pull_down_enable)
    : pin_(pin), mode_(mode), pull_up_enable_(pull_up_enable), pull_down_enable_(pull_down_enable), state_(false) {
}

/**
 * @brief Destructor for the Gpio class.
 */
Gpio::~Gpio() {
    // Reset the pin to default state
    gpio_reset_pin(pin_);
}

/**
 * @brief Initialize the GPIO pin.
 */
void Gpio::init() {
    // Configure the GPIO pin
    gpio_config_t io_conf;
    io_conf.pin_bit_mask = (1ULL << pin_);
    io_conf.mode = mode_;
    io_conf.pull_up_en = pull_up_enable_ ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = pull_down_enable_ ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    // Initialize the state
    if (mode_ == GPIO_MODE_OUTPUT) {
        state_ = false;
        gpio_set_level(pin_, state_);
    }
}

/**
 * @brief Write a state to the GPIO pin.
 * @param state The state to write (true = high, false = low).
 */
void Gpio::write(bool state) {
    state_ = state;
    gpio_set_level(pin_, state_);
}

/**
 * @brief Read the current state of the GPIO pin.
 * @return The current state of the GPIO pin (true = high, false = low).
 */
bool Gpio::read() {
    if (mode_ == GPIO_MODE_INPUT) {
        state_ = gpio_get_level(pin_);
    }
    return state_;
}

/**
 * @brief Toggle the current state of the GPIO pin.
 */
void Gpio::toggle() {
    state_ = !state_;
    gpio_set_level(pin_, state_);
}

/**
 * @brief Initialize interruopt and attach interrupt funtion to the gpio pin
 * 
 */
void Gpio::attachInterrupt(gpio_num_t gpio_pin, gpio_isr_t handler){
    gpio_install_isr_service(0);
    gpio_isr_handler_add(gpio_pin, handler, NULL);
}