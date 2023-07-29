#include "WebSocket.h"

#include <stdio.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"

// #include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_websocket_client.h"
#include "WiFi.h"
#include <vector>

namespace websocket{

    bool isInitilalized = false;
    static const char *TAG = "WEBSOCKET";
    esp_websocket_client_handle_t client;
    esp_websocket_client_config_t websocket_cfg = {};

    //void(*eventHandler)(esp_websocket_event_data_t*) {nullptr};
    std::vector<void(*)(esp_websocket_event_data_t*)> eventHandlers;

    void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
    {
        esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
        switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
            break;
        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
            break;
        case WEBSOCKET_EVENT_DATA:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
            ESP_LOGI(TAG, "Received opcode=%d", data->op_code);
            if (data->op_code == 0x08 && data->data_len == 2) {
                ESP_LOGW(TAG, "Received closed message with code=%d", 256*data->data_ptr[0] + data->data_ptr[1]);
            } else {
                ESP_LOGW(TAG, "Received=%.*s", data->data_len, (char *)data->data_ptr);
                if(!eventHandlers.empty()){
                    for(auto handler: eventHandlers){
                        handler(data);
                    }
                }
                // if(eventHandler != nullptr){
                //     eventHandler(data);
                // }
            }
            ESP_LOGW(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);

            break;
        case WEBSOCKET_EVENT_ERROR:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
            break;
        }
    }

    void setupWebSocket(void(*_eventHandler)(esp_websocket_event_data_t*data)){      
        websocket_cfg.uri = WEBSOCKET_URI;
        //eventHandler = _eventHandler;
        eventHandlers.push_back(_eventHandler);
    }

    void openWebSocket(){
        if(!isInitilalized){
            wifi::setupWiFi();
            wifi::connect();
            
            ESP_LOGI(TAG, "Connecting to %s...", websocket_cfg.uri);

            client = esp_websocket_client_init(&websocket_cfg);
            esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
            esp_websocket_client_start(client);
            isInitilalized = true;
        }
    }

    void closeWebSocket(){
        esp_websocket_client_close(client, portMAX_DELAY);
        ESP_LOGI(TAG, "Websocket Stopped");
        esp_websocket_client_destroy(client);
        wifi::disconnect();
    }

    void sendData(char* data, int len){
        if (esp_websocket_client_is_connected(client)) {
            ESP_LOGW(TAG, "Sending %s", data);
            esp_websocket_client_send_text(client, data, len, portMAX_DELAY);
        }
    }

    void sendDataADC2Clear(char* data, int len){
        openWebSocket();
        vTaskDelay(100 / portTICK_PERIOD_MS);
        if (esp_websocket_client_is_connected(client)) {
            ESP_LOGW(TAG, "Sending %s", data);
            esp_websocket_client_send_text(client, data, len, portMAX_DELAY);
        }
        closeWebSocket();
    }
}