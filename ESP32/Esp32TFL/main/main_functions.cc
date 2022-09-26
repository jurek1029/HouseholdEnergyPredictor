
#include <time.h>
#include <sys/time.h>

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "main_functions.h"
#include "model.h"
// #include "constants.h"
// #include "output_handler.h"
#include "ExponentialSmoothing.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "driver/adc.h"
#include "esp_system.h"
#include "esp_event.h"

#include "protocol_examples_common.h"
#include "esp_sntp.h"

#include "dht11.h"


#define ADC2_CHANNEL    ADC2_CHANNEL_3
#define NO_OF_SAMPLES   64          //Multisampling

// Globals, used for compatibility with Arduino-style sketches.
namespace {
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

    time_t now;
    struct tm timeinfo;
    //float v = 0;
}  // namespace

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

 std::unique_ptr<float[]> invokeModel(float* inputData){
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

  std::unique_ptr<float[]> outData( new float[outputSize]);
  for(int i = 0; i < outputSize; i++){
     float f = (float(output->data.int8[i]) - output->params.zero_point) * output->params.scale;
     outData[i] = f;
    //float f = output->data.f[i];
    printf("%f, \t",f);
    if((i-3) % 4 == 0) printf("\n");
  }

  return outData;
}

void setupADC2(adc2_channel_t channel){
    // esp_err_t err;
    // gpio_num_t adc_gpio_num, dac_gpio_num;
    // err = adc2_pad_get_io_num( ADC2_EXAMPLE_CHANNEL, &adc_gpio_num );
    // assert( err == ESP_OK );
    // printf("ADC2 channel %d @ GPIO %d \n", ADC2_EXAMPLE_CHANNEL, adc_gpio_num);

    //be sure to do the init before using adc2. 
    printf("adc2_init...\n");
    adc2_config_channel_atten( channel, ADC_ATTEN_11db);
}

uint32_t readAnalogADC2(adc2_channel_t channel){
    esp_err_t err;
    int read_raw;

    uint32_t adc_reading = 0;
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
            err = adc2_get_raw(channel, ADC_WIDTH_BIT_12, &read_raw);
            if ( err == ESP_ERR_INVALID_STATE ) {
                printf("%s: ADC2 not initialized yet.\n", esp_err_to_name(err));
                return 0;
            } else if (err == ESP_ERR_TIMEOUT) {
                printf("%s: ADC2 is in use by Wi-Fi.\n", esp_err_to_name(err));
                return 0;
            } else if (err != ESP_OK) {
                printf("%s\n", esp_err_to_name(err));
                return 0;
            }
            adc_reading += read_raw;
        }
        adc_reading /= NO_OF_SAMPLES;

        return adc_reading;
}

static void obtain_time(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        printf("Waiting for system time to be set... (%d/%d)\n", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    ESP_ERROR_CHECK(example_disconnect());
}

void setupNTPTime(){
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2020 - 1900)) {
        printf("Time is not set yet. Connecting to WiFi and getting time over NTP. \n");
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }
    setenv("TZ", "UTC-2", 1);
    tzset();
}

void setup() {

    //expSmoothing.removeFromNVS();
    setupTFLite();
    float in[inputSize] = {0};
    auto outData = invokeModel(in);
    setupNTPTime();
    setupADC2(ADC2_CHANNEL);

    DHT11_init(GPIO_NUM_2);
}
char strftime_buf[64];
void loop() {
    uint32_t val = readAnalogADC2(ADC2_CHANNEL);
    float expValue = expSmoothing.next(val);
    expSmoothing.saveAll();


    printf("%d\n",val);
    dht11_reading dth11_val = DHT11_read();
    printf("Temperature is %f \n", dth11_val.temperature);
    printf("Humidity is %f\n", dth11_val.humidity);

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    printf("Day percent: %f \n", timeinfo.tm_hour / 24.0f + timeinfo.tm_min / (24.0f*60));
    printf("Week percent: %f \n", timeinfo.tm_hour / (24.0f * 7) + timeinfo.tm_min / (24.0f*60 * 7) + timeinfo.tm_wday / 7.0f);
    printf("Year percent: %f \n", timeinfo.tm_yday / (365.0f));

    vTaskDelay(1000 / portTICK_PERIOD_MS);
}
