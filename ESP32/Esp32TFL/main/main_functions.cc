
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

void setup() {

    //expSmoothing.removeFromNVS();
    setupTFLite();
    float in[inputSize] = {0};
    auto outData = invokeModel(in);
    setupADC2(ADC2_CHANNEL);

    DHT11_init(GPIO_NUM_2);
}

void loop() {
    //v = expSmoothing.next(v);
    //expSmoothing.saveAll();
    uint32_t val = readAnalogADC2(ADC2_CHANNEL);
    printf("%d\n",val);
    dht11_reading dth11_val = DHT11_read();
    printf("Temperature is %f \n", dth11_val.temperature);
    printf("Humidity is %f\n", dth11_val.humidity);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

// static bool example_adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle)
// {
//     adc_cali_handle_t handle = NULL;
//     esp_err_t ret = ESP_FAIL;
//     bool calibrated = false;

// #if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
//     if (!calibrated) {
//         ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
//         adc_cali_curve_fitting_config_t cali_config = {
//             .unit_id = unit,
//             .atten = atten,
//             .bitwidth = ADC_BITWIDTH_DEFAULT,
//         };
//         ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
//         if (ret == ESP_OK) {
//             calibrated = true;
//         }
//     }
// #endif

// #if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
//     if (!calibrated) {
//         ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
//         adc_cali_line_fitting_config_t cali_config = {
//             .unit_id = unit,
//             .atten = atten,
//             .bitwidth = ADC_BITWIDTH_DEFAULT,
//         };
//         ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
//         if (ret == ESP_OK) {
//             calibrated = true;
//         }
//     }
// #endif

//     *out_handle = handle;
//     if (ret == ESP_OK) {
//         ESP_LOGI(TAG, "Calibration Success");
//     } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
//         ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
//     } else {
//         ESP_LOGE(TAG, "Invalid arg or no memory");
//     }

//     return calibrated;
// }

// static void example_adc_calibration_deinit(adc_cali_handle_t handle)
// {
// #if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
//     ESP_LOGI(TAG, "deregister %s calibration scheme", "Curve Fitting");
//     ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

// #elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
//     ESP_LOGI(TAG, "deregister %s calibration scheme", "Line Fitting");
//     ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
// #endif
// }