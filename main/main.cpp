#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "driver/gpio.h"
#include "esp_log.h"

#include <Gpio.h>

Gpio led(GPIO_NUM_2, GPIO_MODE_OUTPUT);

extern "C" void app_main(void)
{

    led.init();

    while(1){

        led.toggle();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
    }

}