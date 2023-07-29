
#include "LoadPrediction.h"
#include "solcast_api.h"
#include "http_request.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

//Default BackupValues
// char capacity[] = "5";
// char tilt[] = "42";
// char azimuth[] = "180";
// char hours[] = "12";
// char lat[] = "-33.856784";
// char lon[] = "151.215297";
// char webServer[] = "api.solcast.com.au";
// char apiKey[] = "q_m_SaOSVkHui2bOMrGgzY6T68TbEpN7";

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

  //solcastData = new solcast::SolcastApiData(webServer,apiKey,lat,lon,capacity,tilt,azimuth,hours);
  std::unique_ptr<solcast::Data> pData = solcast::requestForecastData();

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