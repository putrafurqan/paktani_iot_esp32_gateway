/**
 * @file main.cpp
 * @brief ESP-IDF IoT Gateway for PAKTANI IOT.
 *        A Modbus Master that connects to WiFi, polls Modbus slaves,
 *        and stores sensor data (with RTC timestamp) in a FIFO queue.
 */

 #include <stdio.h>
 #include <string.h>
 #include <time.h>
 
 #include "freertos/FreeRTOS.h"
 #include "freertos/task.h"
 #include "freertos/queue.h"
 #include "sdkconfig.h"
 #include "esp_log.h"
 
 #include "Wifi.h"
 #include "DS3231.h"
 #include "I2CMaster.h"
 #include "Modbus.h"
 #include "Gpio.h"
 
 // Tag for logging
 #define TAG "MAIN"

 #define CONFIG_MB_UART_BAUD_RATE 115200
 
 // Size of the FIFO queue for sensor data
 #define SENSOR_QUEUE_LENGTH 50
 
 // Structure to hold sensor data with timestamp
 typedef struct {
     struct tm timestamp;  // RTC timestamp
     uint8_t slave_id;     // Modbus slave ID
     uint16_t dev_status;  // Device status (1 register)
     float humidity;       // Humidity (converted from 2 registers)
     float temperature;    // Temperature (converted from 2 registers)
 } SensorRecord;
 
 // Global FIFO queue handle for sensor data
 QueueHandle_t sensorDataQueue = NULL;
 
 // Global flag for WiFi connection status (can trigger mode change)
 volatile bool wifiConnected = false;
 
 // Forward declarations of tasks
 void wifiTask(void *pvParameters);
 void modbusTask(void *pvParameters);
 void ledTask(void *pvParameters);
 
 // Global LED instance (using GPIO2 as example)
 Gpio led(GPIO_NUM_2, GPIO_MODE_OUTPUT);
 
 // Helper function: Convert two 16-bit registers to float (IEEE 754, big-endian)
 float convertRegistersToFloat(uint16_t high, uint16_t low) {
     uint32_t combined = ((uint32_t)high << 16) | low;
     float value;
     memcpy(&value, &combined, sizeof(value));
     return value;
 }
 
 // Task to initialize and maintain WiFi connection
 void wifiTask(void *pvParameters) {
     Wifi wifi;
     // Set your WiFi credentials here
     wifi.setSSID("SSID");
     wifi.setPassword("PASSWORD");
 
     if (!wifi.init()) {
         ESP_LOGE(TAG, "WiFi initialization failed");
     }
     if (!wifi.connect()) {
         ESP_LOGE(TAG, "WiFi connection failed");
         wifiConnected = false;
     } else {
         ESP_LOGI(TAG, "WiFi connected");
         wifiConnected = true;
     }
 
     // Monitor WiFi connection in a loop
     while (1) {
         // Here you might call a function to re-check or reconnect if necessary.
         if (!wifiConnected) {
             ESP_LOGW(TAG, "WiFi disconnected! Triggering status change.");
             // Add any additional actions for WiFi loss here.
         }
         vTaskDelay(5000 / portTICK_PERIOD_MS);
     }
 }
 
 // Task to poll Modbus slaves, get RTC time, and store the data in a FIFO queue
 void modbusTask(void *pvParameters) {
     // Initialize the I2C master and RTC (DS3231)
     I2CMaster i2c_master(I2C_NUM_0);
     DS3231 rtc(&i2c_master);
     if (rtc.init() != ESP_OK) {
         ESP_LOGE(TAG, "RTC initialization failed");
     }
 
     ModbusRTU modbus1(MB_DEVICE_ADDR1, UART_NUM_1, MB_DEV_SPEED, UART_PARITY_DISABLE, MB_MODE_RTU, 17, 16, -1);
     ModbusRTU modbus2(MB_DEVICE_ADDR2, UART_NUM_1, MB_DEV_SPEED, UART_PARITY_DISABLE, MB_MODE_RTU, 17, 16, -1);
     ModbusRTU modbus3(MB_DEVICE_ADDR3, UART_NUM_1, MB_DEV_SPEED, UART_PARITY_DISABLE, MB_MODE_RTU, 17, 16, -1);
 
     // Initialize each Modbus interface
     if (!modbus1.init()) {
         ESP_LOGE(TAG, "Modbus init failed for slave 1");
     }
     if (!modbus2.init()) {
         ESP_LOGE(TAG, "Modbus init failed for slave 2");
     }
     if (!modbus3.init()) {
         ESP_LOGE(TAG, "Modbus init failed for slave 3");
     }
 
     uint16_t response[5]; // To hold the five registers read from a slave
     SensorRecord record;
     struct tm currentTime;
 
     while (1) {
         // Loop over each modbus slave
         for (int i = 0; i < 3; i++) {
             ModbusRTU* modbus = NULL;
             uint8_t slave_id = 0;
             if (i == 0) {
                 modbus = &modbus1;
                 slave_id = MB_DEVICE_ADDR1;
             } else if (i == 1) {
                 modbus = &modbus2;
                 slave_id = MB_DEVICE_ADDR2;
             } else if (i == 2) {
                 modbus = &modbus3;
                 slave_id = MB_DEVICE_ADDR3;
             }
 
             // Get current time from the RTC
             if (rtc.getTime(&currentTime) != ESP_OK) {
                 ESP_LOGE(TAG, "Failed to get RTC time");
             }
 
             // Read 5 holding registers starting at address 8:
             // [0]: device status, [1-2]: humidity, [3-4]: temperature.
             if (modbus->readHoldingRegisters(8, 5, response)) {
                 record.slave_id = slave_id;
                 record.timestamp = currentTime;
                 record.dev_status = response[0];
                 record.humidity = convertRegistersToFloat(response[1], response[2]);
                 record.temperature = convertRegistersToFloat(response[3], response[4]);
 
                 // Enqueue the sensor record into the FIFO queue
                 if (xQueueSend(sensorDataQueue, &record, 0) != pdPASS) {
                     ESP_LOGW(TAG, "Sensor data queue full, record dropped");
                 } else {
                     ESP_LOGI(TAG, "Recorded data from slave %d", slave_id);
                 }
             } else {
                 ESP_LOGE(TAG, "Modbus read failed for slave %d", slave_id);
             }
 
             vTaskDelay(POLL_TIMEOUT_TICS);
         }
         vTaskDelay(1000 / portTICK_PERIOD_MS);
     }
 }
 
 // Task to toggle LED for visual feedback
 void ledTask(void *pvParameters) {
     while (1) {
         led.toggle();
         vTaskDelay(1000 / portTICK_PERIOD_MS);
     }
 }
 
 extern "C" void app_main(void)
 {
     // Initialize the LED
     led.init();
 
     // Create the sensor data FIFO queue
     sensorDataQueue = xQueueCreate(SENSOR_QUEUE_LENGTH, sizeof(SensorRecord));
     if (sensorDataQueue == NULL) {
         ESP_LOGE(TAG, "Failed to create sensor data queue");
     }
 
     // Create the WiFi, Modbus, and LED tasks
     xTaskCreate(wifiTask, "wifiTask", 4096, NULL, 5, NULL);
     xTaskCreate(modbusTask, "modbusTask", 8192, NULL, 5, NULL);
     xTaskCreate(ledTask, "ledTask", 2048, NULL, 5, NULL);
 
     SensorRecord rec;
     while (1) {
         if (xQueueReceive(sensorDataQueue, &rec, portMAX_DELAY) == pdPASS) {
             ESP_LOGI(TAG, "Data from slave %d: status=%d, humidity=%.2f, temp=%.2f at %02d:%02d:%02d",
                      rec.slave_id, rec.dev_status, rec.humidity, rec.temperature,
                      rec.timestamp.tm_hour, rec.timestamp.tm_min, rec.timestamp.tm_sec);
         }
     }
 }
 