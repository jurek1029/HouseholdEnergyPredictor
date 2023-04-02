#include "LoadPrediction.h"

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "ExponentialSmoothing.h"
#include "model.h"
#include "NTPTime.h"
#include "ADCHelper.h"
#include "dht11.h"
#include "WebSocket.h"
// #include "WiFi.h"

#include <stdio.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_websocket_client.h"

//#define ADC2_CHANNEL    ADC2_CHANNEL_3
#define LOAD_CHANNEL    ADC1_CHANNEL_0
#define DHT_CHANNEL     GPIO_NUM_4
#define INPUT_LEN 40
#define OUTPUT_LEN 16
#define STORAGE_NAMESPACE "storage"

namespace LoadPrediction{

    tflite::ErrorReporter* error_reporter = nullptr;
    const tflite::Model* model = nullptr;
    tflite::MicroInterpreter* interpreter = nullptr;
    TfLiteTensor* input = nullptr;
    TfLiteTensor* output = nullptr;
    int inputSize = 1;
    int outputSize = 1;

    constexpr int kTensorArenaSize = 8000;
    uint8_t tensor_arena[kTensorArenaSize];

    ExpSmoothing expSmoothing;

    float pastEnergyValues[INPUT_LEN] = {0};
    float pastExpSmoothValues[INPUT_LEN] = {0};
    float pastTempValues[INPUT_LEN] = {0};
    float pastWeekPercValues[INPUT_LEN] = {0};

    esp_err_t loadValuesFromNVS(const char* name, void* value){  
        nvs_handle_t  handle;
        esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
        if (err != ESP_OK) return err;

        size_t required_size = 0; 
        err = nvs_get_blob(handle, name, NULL, &required_size);
        if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;

        if (required_size > 0) {
            printf("req size: %s", name);
            err = nvs_get_blob(handle, name, value, &required_size);
            if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND){
                return err;  
            } 
        }
        nvs_close(handle);
        return ESP_OK;
    }

    esp_err_t saveValuesToNVS(const char* name, size_t required_size, void* value){
        nvs_handle_t  handle;
        esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
        if (err != ESP_OK) return err;
        
        err = nvs_set_blob(handle, name, value, required_size);
        if (err != ESP_OK) return err;

        // Commit
        err = nvs_commit(handle);
        if (err != ESP_OK) return err;

        // Close
        nvs_close(handle);
        return ESP_OK;
    }

    void moveValuesToPast(){
        for(int i = 1; i < INPUT_LEN; i ++){
            pastEnergyValues[i - 1] = pastEnergyValues[i];
            pastExpSmoothValues[i - 1] = pastExpSmoothValues[i];
            pastTempValues[i - 1] = pastTempValues[i];
            pastWeekPercValues[i - 1] = pastWeekPercValues[i];
        }
    }

    void saveAllValues(){
        saveValuesToNVS("energy_values",sizeof(pastEnergyValues) ,pastEnergyValues);
        saveValuesToNVS("exp_values", sizeof(pastExpSmoothValues), pastExpSmoothValues);
        saveValuesToNVS("temp_values", sizeof(pastTempValues), pastTempValues);
        saveValuesToNVS("week_values",sizeof(pastWeekPercValues), pastWeekPercValues);
        expSmoothing.saveAll();
        #ifdef DEBUG_PRINT_LOGS
        printf("LoadPrediction Saving Values \n");
        #endif
    }

    void setupTFLite(){
        static tflite::MicroErrorReporter micro_error_reporter;
        error_reporter = &micro_error_reporter;

        model = tflite::GetModel(g_model);
        if (model->version() != TFLITE_SCHEMA_VERSION) {
            TF_LITE_REPORT_ERROR(error_reporter, 
                                "Model provided is schema version %d not equal "
                                "to supported version %d.",
                                model->version(), TFLITE_SCHEMA_VERSION);
            return;
        }

        static tflite::AllOpsResolver resolver;
        static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
        interpreter = &static_interpreter;

        // Allocate memory from the tensor_arena for the model's tensors.
        TfLiteStatus allocate_status = interpreter->AllocateTensors();
        if (allocate_status != kTfLiteOk) {
            TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
            return;
        }

        // Obtain pointers to the model's input and output tensors.
        input = interpreter->input(0);
        output = interpreter->output(0);

        printf("size: %d \n",input->dims->size);
        for(int i = 0; i < input->dims->size; i++){
            inputSize *= input->dims->data[i];
            printf("dim%d : %d\n",i,input->dims->data[i]);
        }
        printf("%d\n",int(input->type));
        printf("\n");

        printf("input: zero point: %d, scale: %f \n", input->params.zero_point, input->params.scale);

        printf("size: %d \n",output->dims->size);
        for(int i = 0; i < output->dims->size; i++){
            outputSize *= output->dims->data[i];
            printf("dim%d : %d\n",i,output->dims->data[i]);
        }
        printf("output: zero point: %d, scale: %f \n", output->params.zero_point, output->params.scale);
    }

    void webSocketMessageHandler(esp_websocket_event_data_t* data){
        printf("Message Handeled: %.*s \n", data->data_len, (char *)data->data_ptr);
    }

    void setupTFLiteLoadPrediction(){
        // std::fill_n(pastExpSmoothValues,INPUT_LEN,1);
        // std::fill_n(pastTempValues,INPUT_LEN,2);
        // std::fill_n(pastWeekPercValues,INPUT_LEN,3);
        loadValuesFromNVS("energy_values", pastEnergyValues);
        loadValuesFromNVS("exp_values", pastExpSmoothValues);
        loadValuesFromNVS("temp_values", pastTempValues);
        loadValuesFromNVS("week_values", pastWeekPercValues);

        #ifdef DEBUG_PRINT_LOGS
        for(int i = 0; i < INPUT_LEN; i++){
            printf("%f %f %f %f \n", pastEnergyValues[i], pastExpSmoothValues[i], pastTempValues[i], pastWeekPercValues[i]);
        }
        #endif

        setupTFLite();
        setupADC1(LOAD_CHANNEL);
        //setupADC2(ADC2_CHANNEL);
        DHT11_init(DHT_CHANNEL);
        NTPTime::setupNTPTime();
        websocket::setupWebSocket(&webSocketMessageHandler);
        websocket::openWebSocket();
    }

    std::shared_ptr<float[]> invokeModel(float* inputData){
        for(int i = 0; i < inputSize; i ++){
            input->data.int8[i] = inputData[i] / input->params.scale + input->params.zero_point;
            //input->data.f[i] = float(1.);
        }
        //printf("x: %d, %d \n", input->data.int8[0], input->data.int8[1]);
        //input->data.int8 = inputData;

        TfLiteStatus invoke_status = interpreter->Invoke();
        if (invoke_status != kTfLiteOk) {
            TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed on x: \n" );
            return nullptr;
        }

        std::shared_ptr<float[]> outData( new float[outputSize]);
        for(int i = 0; i < outputSize; i++){
            float f = (float(output->data.int8[i]) - output->params.zero_point) * output->params.scale;
            outData[i] = f;
            //float f = output->data.f[i];
            #ifdef DEBUG_PRINT_LOGS
            printf("%f, \t",f);
            if((i-3) % 4 == 0) printf("\n");
            #endif
        }

        return outData;
    }

    std::shared_ptr<float[]> invokeModel(float* inputDataEnergy, float* inputDataExpSmooth, float* inputDataTemp, float* inputDataWeekPer){
        for(int i = 0; i < inputSize / 4; i ++){
            input->data.int8[i * 4]     = inputDataEnergy[i] / input->params.scale + input->params.zero_point;
            input->data.int8[i * 4 + 1] = inputDataExpSmooth[i] / input->params.scale + input->params.zero_point;
            input->data.int8[i * 4 + 2] = inputDataTemp[i] / input->params.scale + input->params.zero_point;
            input->data.int8[i * 4 + 3] = inputDataWeekPer[i] / input->params.scale + input->params.zero_point;
        }

        TfLiteStatus invoke_status = interpreter->Invoke();
        if (invoke_status != kTfLiteOk) {
            TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed on x: \n" );
            return nullptr;
        }

        std::shared_ptr<float[]> outData( new float[outputSize]);
        for(int i = 0; i < outputSize; i++){
            float f = (float(output->data.int8[i]) - output->params.zero_point) * output->params.scale;
            outData[i] = f;
            //float f = output->data.f[i];
            #ifdef DEBUG_PRINT_LOGS
            printf("%f, \t",f);
            if((i-3) % 4 == 0) printf("\n");
            #endif
        }

        return outData;
    }

    void sendDataToServer(float load, float temp, float humi, std::shared_ptr<float[]> prediction){
        //TODO temp opening and closing waiting for beter esp32 with free adc1 pin
        //websocket::openWebSocket();
        vTaskDelay(300 / portTICK_PERIOD_MS);
        char data[OUTPUT_LEN*8 + 40];
        int len = sprintf(data, "{\"type\":\"load\",\"value\":%f}", load);
        websocket::sendData(data,len);

        len = sprintf(data, "{\"type\":\"temp\",\"value\":%f}", temp);
        websocket::sendData(data,len);

        len = sprintf(data, "{\"type\":\"humi\",\"value\":%f}", humi);
        websocket::sendData(data,len);

        char s_pred[OUTPUT_LEN*8 + 8];

        int j = 0;
        for (int i = 0; i < OUTPUT_LEN; i++){
            if(i != OUTPUT_LEN - 1){
                j += sprintf(s_pred+j, "%6.4f,",prediction[i]);
            }
            else{
                j += sprintf(s_pred+j, "%6.4f",prediction[i]);
            }
        }

        #ifdef DEBUG_PRINT_LOGS
        printf("formated values to JSON: %s\n", s_pred);
        #endif

        len = sprintf(data, "{\"type\":\"pred\",\"value\":[%s]}", s_pred);
        websocket::sendData(data,len);
        //vTaskDelay(200 / portTICK_PERIOD_MS);
        //websocket::closeWebSocket();
    }

    std::shared_ptr<float[]> predictNextLoad(){

        moveValuesToPast();

        // TODO Read real power demand in kWh
        uint32_t val = readAnalogADC1(LOAD_CHANNEL);
        
        float expValue = expSmoothing.next(val);

        dht11_reading dth11_val = DHT11_read();

        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        float weekPercent = timeinfo.tm_hour / (24.0f * 7) + timeinfo.tm_min / (24.0f*60 * 7) + timeinfo.tm_wday / 7.0f;

        pastEnergyValues[INPUT_LEN - 1] = val;
        pastExpSmoothValues[INPUT_LEN - 1] = expValue;
        pastTempValues[INPUT_LEN - 1] = dth11_val.temperature;
        pastWeekPercValues[INPUT_LEN - 1] = weekPercent;

        #ifdef DEBUG_PRINT_LOGS
        printf("val: %d  expVal: %f\n",val, expValue);
        printf("Temperature is %f \n", dth11_val.temperature);
        printf("Humidity is %f\n", dth11_val.humidity);
        printf("Day percent: %f \n", timeinfo.tm_hour / 24.0f + timeinfo.tm_min / (24.0f*60));
        printf("Week percent: %f \n", timeinfo.tm_hour / (24.0f * 7) + timeinfo.tm_min / (24.0f*60 * 7) + timeinfo.tm_wday / 7.0f);
        printf("Year percent: %f \n", timeinfo.tm_yday / (365.0f));
        #endif
        
        auto outData = invokeModel(pastEnergyValues, pastExpSmoothValues, pastTempValues, pastWeekPercValues);
        saveAllValues();
        
        sendDataToServer((float)val, dth11_val.temperature, dth11_val.humidity, outData);

        return outData;
    } 

    void removeFromNVS(){
        nvs_flash_erase();
    }
}