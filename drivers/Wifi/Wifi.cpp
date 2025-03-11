#include "Wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

// Constructor
Wifi::Wifi() {
    // Initialize WiFi configuration
    memset(&this->wifi_config, 0, sizeof(wifi_config_t));
    this->s_retry_num = 0;
    this->s_wifi_event_group = xEventGroupCreate();
}

// Destructor
Wifi::~Wifi() {
    // Clean up resources if needed
    if (this->s_wifi_event_group != nullptr) {
        vEventGroupDelete(this->s_wifi_event_group);
    }
}

// Initialize WiFi
bool Wifi::init() {
    // Initialize NVS (Non-Volatile Storage)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize TCP/IP stack and default event loop
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create default WiFi station interface
    esp_netif_create_default_wifi_sta();

    // Initialize WiFi with default configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register event handlers
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                      ESP_EVENT_ANY_ID,
                                                      &Wifi::event_handler,
                                                      this,
                                                      &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                      IP_EVENT_STA_GOT_IP,
                                                      &Wifi::event_handler,
                                                      this,
                                                      &instance_got_ip));

    return true;
}

// Connect to the configured WiFi network
bool Wifi::connect() {
    // Set WiFi mode to station
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // Set WiFi configuration
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &this->wifi_config));

    // Start WiFi
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi initialization finished.");

    // Wait for connection or failure
    EventBits_t bits = xEventGroupWaitBits(this->s_wifi_event_group,
                                          WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                          pdFALSE,
                                          pdFALSE,
                                          portMAX_DELAY);

    // Check connection status
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to AP SSID: %s", this->wifi_config.sta.ssid);
        return true;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID: %s", this->wifi_config.sta.ssid);
        return false;
    } else {
        ESP_LOGE(TAG, "Unexpected event");
        return false;
    }
}

// Set the SSID for the WiFi network
void Wifi::setSSID(const std::string& ssid) {
    strncpy((char*)this->wifi_config.sta.ssid, ssid.c_str(), sizeof(this->wifi_config.sta.ssid));
}

// Set the password for the WiFi network
void Wifi::setPassword(const std::string& password) {
    strncpy((char*)this->wifi_config.sta.password, password.c_str(), sizeof(this->wifi_config.sta.password));
}

// Event handler for WiFi events
void Wifi::event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    Wifi* wifi = static_cast<Wifi*>(arg);

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (wifi->s_retry_num < wifi->MAXIMUM_RETRY) {
            esp_wifi_connect();
            wifi->s_retry_num++;
            ESP_LOGI(TAG, "Retry to connect to the AP");
        } else {
            xEventGroupSetBits(wifi->s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "Connect to the AP failed");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        wifi->s_retry_num = 0;
        xEventGroupSetBits(wifi->s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}