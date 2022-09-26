
#include "main_functions.h"
#include "LoadPrediction.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void setup() {

    //LoadPrediction::removeFromNVS();
    LoadPrediction::setupTFLiteLoadPrediction();
    float in[4*40] = {0};
    auto outData = LoadPrediction::invokeModel(in);
}

void loop() {
    LoadPrediction::predictNextLoad();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}
