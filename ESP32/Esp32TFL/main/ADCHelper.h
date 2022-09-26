#ifndef ADC_HELPER_H_
#define ADC_HELPER_H_

#include "driver/adc.h"

#define NO_OF_SAMPLES   64          //Multisampling
//namespace ADC{
    void setupADC2(adc2_channel_t channel);
    uint32_t readAnalogADC2(adc2_channel_t channel);
//}
#endif  // ADC_HELPER_H_
