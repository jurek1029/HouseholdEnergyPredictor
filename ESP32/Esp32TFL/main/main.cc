
#include "LoadPrediction.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

void setup(){
    //LoadPrediction::removeFromNVS();
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("WEBSOCKET_CLIENT", ESP_LOG_DEBUG);
    esp_log_level_set("TRANSPORT_WS", ESP_LOG_DEBUG);
    esp_log_level_set("TRANS_TCP", ESP_LOG_DEBUG);

    esp_log_level_set("wifi", ESP_LOG_ERROR);
    esp_log_level_set("wifi_init", ESP_LOG_ERROR);
    esp_log_level_set("example_connect", ESP_LOG_ERROR);

    esp_log_level_set("WEBSOCKET", ESP_LOG_WARN);
    LoadPrediction::setupTFLiteLoadPrediction();
    float in[4*40] = {0};
    auto outData = LoadPrediction::invokeModel(in);
}

void loop(){
    auto outData = LoadPrediction::predictNextLoad();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
}


extern "C" void app_main(void) {
  setup();
  while (true) {
    loop();
  }
}