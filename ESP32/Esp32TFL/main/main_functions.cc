/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/


#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "main_functions.h"
#include "model.h"
#include "constants.h"
#include "output_handler.h"

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
  float v = 0;
  //int inference_count = 0;

  // constexpr int kTensorArenaSize = 49016;
  // constexpr int kTensorArenaSize = 16000;
  constexpr int kTensorArenaSize = 8000;
  uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

void setup() {

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
    printf("dim%d : %d\n",i,input->dims->data[i]);
  }
  printf("%d\n",int(input->type));
  printf("\n");

  //float* inputData = interpreter->typed_input_tensor<float>(0);

  // if (inputData == nullptr){
  //   printf("inputData is nullptr \n");
  // }
  printf("input: zero point: %d, scale: %f \n", input->params.zero_point, input->params.scale);
  // for(int i = 0; i < 40*4; i ++){
  //   inputData[i] = input->params.zero_point;
  // }
  for(int i = 0; i < 40*4; i ++){
     input->data.int8[i] = test0[i] / input->params.scale + input->params.zero_point;
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
  printf("size: %d \n",output->dims->size);
  for(int i = 0; i < output->dims->size; i++){
    printf("dim%d : %d\n",i,output->dims->data[i]);
  }
  printf("output: zero point: %d, scale: %f \n", output->params.zero_point, output->params.scale);
  for(int i = 0; i < 16; i++){
     float f = (float(output->data.int8[i]) - output->params.zero_point) * output->params.scale;
    //float f = output->data.f[i];
    printf("%f, \t",f);
    if((i-3) % 4 == 0) printf("\n");
  }

  // for(int i = 0; i < 16; i++){
  //    printf("%d, \t",output->data.int8[i]);
  // }
}

void loop() {
  
  if (v < 5){
    for(int i = 0; i < 40*4; i ++){
      input->data.int8[i] = v / input->params.scale + input->params.zero_point;
      //input->data.f[i] = float(v);
    }

    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
      TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed on x: \n" );
      return;
    }

    output = interpreter->output(0);
    printf("v: %f \n", v);
    for(int i = 0; i < 16; i++){
      float f = (float(output->data.int8[i]) - output->params.zero_point) * output->params.scale;
      //float f = output->data.f[i];
      printf("%f, \t",f);
      if((i-3) % 4 == 0) printf("\n");
    }
    printf("\n");

    // for(int i = 0; i < 16; i++){
    //    printf("%d, \t",output->data.int8[i]);
    // }
    v ++;
  }
  else{
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  // //int8_t data[1][40][4];

  // // for ( int i = 0; i < 40; i++){
  // //   for (int j = 0; j < 4; j++) {
  // //       data[0][i][j] = inputData[0][i][j] / input->params.scale + input->params.zero_point;
  // //   }
  // // }
  // // Quantize the input from floating-point to integer
  // //int8_t x_quantized = inputData / input->params.scale + input->params.zero_point;
  // // Place the quantized input in the model's input tensor
  // //input->data.int8[0] = inputData;

  // // Run inference, and report any error
  // TfLiteStatus invoke_status = interpreter->Invoke();
  // if (invoke_status != kTfLiteOk) {
  //   TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed on x: \n" );
  //   return;
  // }

  // int8_t* output = interpreter->typed_output_tensor<int8_t>(0);
  // printf("%d", output[0]);
  // // Obtain the quantized output from model's output tensor
  // // int8_t y_quantized = output->data.int8[0];
  // // // Dequantize the output from integer to floating-point
  // // float y = (y_quantized - output->params.zero_point) * output->params.scale;

  // // Output the results. A custom HandleOutput function can be implemented
  // // for each supported hardware target.
  // //HandleOutput(error_reporter, x, y);

  // // Increment the inference_counter, and reset it if we have reached
  // // the total number per cycle
  // // inference_count += 1;
  // // if (inference_count >= kInferencesPerCycle) inference_count = 0;
}
