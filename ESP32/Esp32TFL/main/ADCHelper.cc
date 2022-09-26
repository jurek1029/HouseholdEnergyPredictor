#include "ADCHelper.h"

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