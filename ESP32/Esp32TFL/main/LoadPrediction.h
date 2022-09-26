#ifndef LOAD_PREDICTION_H_
#define LOAD_PREDICTION_H_

#include <memory>

#define DEBUG_PRINT_LOGS

namespace LoadPrediction{

    void setupTFLiteLoadPrediction();
    std::unique_ptr<float[]> invokeModel(float* inputData);
    std::unique_ptr<float[]> predictNextLoad(); 
    void removeFromNVS();
}
#endif  // LOAD_PREDICTION_H_
