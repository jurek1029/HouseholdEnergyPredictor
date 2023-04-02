
#include "LoadPrediction.h"
#include "solcast_api.h"
#include "http_request.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

const char capacity[] = "5";
const char tilt[] = "42";
const char azimuth[] = "180";
const char hours[] = "12";
const char lat[] = "-33.856784";
const char lon[] = "151.215297";

void setupLogsLevels(){
 	esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("WEBSOCKET_CLIENT", ESP_LOG_DEBUG);
    esp_log_level_set("TRANSPORT_WS", ESP_LOG_DEBUG);
    esp_log_level_set("TRANS_TCP", ESP_LOG_DEBUG);

    esp_log_level_set("wifi", ESP_LOG_ERROR);
    esp_log_level_set("wifi_init", ESP_LOG_ERROR);
    esp_log_level_set("example_connect", ESP_LOG_ERROR);

    esp_log_level_set("WEBSOCKET", ESP_LOG_WARN);
    esp_log_level_set("time_sync", ESP_LOG_WARN);
}

void setup(){
    //LoadPrediction::removeFromNVS();
   	setupLogsLevels();

	solcast::setupSolCastApi();
	std::unique_ptr<solcast::Data> pData = solcast::requestForecastData(
		solcast::SolcastApiData(lat,lon,capacity,tilt,azimuth,hours));

    LoadPrediction::setupTFLiteLoadPrediction();
    float in[4*40] = {0};
    auto outData = LoadPrediction::invokeModel(in);
	printData(*pData);
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