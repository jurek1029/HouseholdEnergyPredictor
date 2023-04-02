#ifndef LOAD_PREDICTION_H_
#define LOAD_PREDICTION_H_

#include <memory>

#define DEBUG_PRINT_LOGS

namespace LoadPrediction{

    void setupTFLiteLoadPrediction();
    std::shared_ptr<float[]> invokeModel(float* inputData);
    std::shared_ptr<float[]> predictNextLoad(); 
    void removeFromNVS();
}
#endif  // LOAD_PREDICTION_H_
