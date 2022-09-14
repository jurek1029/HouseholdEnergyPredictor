
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


// Globals, used for compatibility with Arduino-style sketches.
namespace {
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* input = nullptr;
  TfLiteTensor* output = nullptr;

  constexpr int kTensorArenaSize = 8000;
  uint8_t tensor_arena[kTensorArenaSize];

  ExpSmoothing expSmoothing;
  float v = 0;
}  // namespace

void setup() {

  //expSmoothing.removeFromNVS();
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

  int inputSize = 1;
  printf("size: %d \n",input->dims->size);
  for(int i = 0; i < input->dims->size; i++){
    inputSize *= input->dims->data[i];
    printf("dim%d : %d\n",i,input->dims->data[i]);
  }
  printf("%d\n",int(input->type));
  printf("\n");

  printf("input: zero point: %d, scale: %f \n", input->params.zero_point, input->params.scale);

  for(int i = 0; i < inputSize; i ++){
     input->data.int8[i] = 0 / input->params.scale + input->params.zero_point;
     //input->data.f[i] = float(1.);
  }
  //printf("x: %d, %d \n", input->data.int8[0], input->data.int8[1]);
  //input->data.int8 = inputData;

  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed on x: \n" );
    return;
  }

  output = interpreter->output(0);
  int outputSize = 1;
  printf("size: %d \n",output->dims->size);
  for(int i = 0; i < output->dims->size; i++){
    outputSize *= output->dims->data[i];
    printf("dim%d : %d\n",i,output->dims->data[i]);
  }
  printf("output: zero point: %d, scale: %f \n", output->params.zero_point, output->params.scale);
  for(int i = 0; i < outputSize; i++){
     float f = (float(output->data.int8[i]) - output->params.zero_point) * output->params.scale;
    //float f = output->data.f[i];
    printf("%f, \t",f);
    if((i-3) % 4 == 0) printf("\n");
  }

}

void loop() {
  expSmoothing.print();
  v = expSmoothing.next(v);
  expSmoothing.saveAll();
  expSmoothing.print();
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  // if (v < 5){
  //   for(int i = 0; i < 40*4; i ++){
  //     input->data.int8[i] = v / input->params.scale + input->params.zero_point;
  //     //input->data.f[i] = float(v);
  //   }

  //   TfLiteStatus invoke_status = interpreter->Invoke();
  //   if (invoke_status != kTfLiteOk) {
  //     TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed on x: \n" );
  //     return;
  //   }

  //   output = interpreter->output(0);
  //   printf("v: %f \n", v);
  //   for(int i = 0; i < 16; i++){
  //     float f = (float(output->data.int8[i]) - output->params.zero_point) * output->params.scale;
  //     //float f = output->data.f[i];
  //     printf("%f, \t",f);
  //     if((i-3) % 4 == 0) printf("\n");
  //   }
  //   printf("\n");

  //   // for(int i = 0; i < 16; i++){
  //   //    printf("%d, \t",output->data.int8[i]);
  //   // }
  //   v ++;
  // }
  // else{
  // vTaskDelay(1000 / portTICK_PERIOD_MS);
  //  }
}
