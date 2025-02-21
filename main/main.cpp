/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "driver/gpio.h"

#define BUILTIN_LED_GPIO ((gpio_num_t)48)

bool state = false;

extern "C" void hardware_init(){
    gpio_reset_pin(BUILTIN_LED_GPIO);
    gpio_set_direction(BUILTIN_LED_GPIO, GPIO_MODE_OUTPUT);
}

extern "C" void app_main(void)
{
    hardware_init();

    while (1)
    {
        ESP_LOGI("MAIN", "Turning the LED %s!", state == true ? "ON" : "OFF");
        state = !state;
        gpio_set_level(BUILTIN_LED_GPIO, state);
        vTaskDelay(500);
    }
    

}
