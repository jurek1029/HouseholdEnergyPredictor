#include "WiFi.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"

#include "protocol_examples_common.h"

namespace wifi{
    static bool initialized = false;
    
    void setupWiFi(){
        if(!initialized){
            ESP_ERROR_CHECK(esp_netif_init());
            ESP_ERROR_CHECK(esp_event_loop_create_default());
            initialized = true;
        }
    }
    void connect(){
        ESP_ERROR_CHECK(example_connect());
    }
    void disconnect(){
        ESP_ERROR_CHECK(example_disconnect());
    }
}