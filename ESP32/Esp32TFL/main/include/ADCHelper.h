#ifndef ADC_HELPER_H_
#define ADC_HELPER_H_

#include "driver/adc.h"

#define NO_OF_SAMPLES   64          //Multisampling
//namespace ADC{
    void setupADC1(adc1_channel_t channel);
    void setupADC2(adc2_channel_t channel);
    bool setupVoltageConversion();
    uint32_t convertToVoltage(uint32_t rawData);
    uint32_t readAnalogADC1(adc1_channel_t channel);
    uint32_t readAnalogADC2(adc2_channel_t channel);
//}
#endif  // ADC_HELPER_H_
