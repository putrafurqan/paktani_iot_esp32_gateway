#ifndef WIFI_H
#define WIFI_H

#include <string>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/event_groups.h"

class Wifi {
public:
    // Constructor
    Wifi();

    // Destructor
    ~Wifi();

    // Initialize WiFi
    bool init();

    // Connect to the configured WiFi network
    bool connect();

    // Set the SSID for the WiFi network
    void setSSID(const std::string& ssid);

    // Set the password for the WiFi network
    void setPassword(const std::string& password);

private:
    // Event handler for WiFi events
    static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

    // Internal function to initialize WiFi station mode
    bool init_sta();

    // WiFi configuration
    wifi_config_t wifi_config;

    // Event group to signal WiFi connection status
    EventGroupHandle_t s_wifi_event_group;

    // Retry count for connection attempts
    int s_retry_num;

    // Constants for event bits
    static constexpr int WIFI_CONNECTED_BIT = BIT0;
    static constexpr int WIFI_FAIL_BIT = BIT1;

    // Maximum number of retries for connection
    static constexpr int MAXIMUM_RETRY = 10;

    // Tag for logging
    static constexpr const char* TAG = "Wifi";
};

#endif // WIFI_H