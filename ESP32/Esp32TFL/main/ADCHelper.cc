#include "ADCHelper.h"
#include "esp_adc_cal.h"
#include "esp_log.h"

#define DEFAULT_VREF    1100 
#define MAX_WAIT_ADC_2  100

static esp_adc_cal_characteristics_t adc1_chars;
static esp_adc_cal_characteristics_t adc2_chars;
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_11db;

const char TAG[] = "ADCHelper";

bool setupVoltageConversion(){
    esp_err_t err;
    bool cali_enable = false;

    err = esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF);
    if (err == ESP_ERR_NOT_SUPPORTED) {
        ESP_LOGW(TAG, "Calibration scheme not supported, skip software calibration");
    } else if (err == ESP_ERR_INVALID_VERSION) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else if (err == ESP_OK) {
        cali_enable = true;
        esp_adc_cal_characterize(ADC_UNIT_1, atten, width, DEFAULT_VREF, &adc1_chars);
        esp_adc_cal_characterize(ADC_UNIT_2, atten, width, DEFAULT_VREF, &adc2_chars);
    } else {
        ESP_LOGE(TAG, "Invalid arg");
    }

    return cali_enable;
}

uint32_t convertToVoltage(uint32_t rawData, bool isADC1){
    if(isADC1){
        return esp_adc_cal_raw_to_voltage(rawData, &adc1_chars);
    }
    else{
        return esp_adc_cal_raw_to_voltage(rawData, &adc2_chars);
    }
}

void setupADC1(adc1_channel_t channel){
    ESP_ERROR_CHECK(adc1_config_width(width));
    ESP_ERROR_CHECK(adc1_config_channel_atten(channel, atten));
}

void setupADC2(adc2_channel_t channel){
    // esp_err_t err;
    // gpio_num_t adc_gpio_num, dac_gpio_num;
    // err = adc2_pad_get_io_num( ADC2_EXAMPLE_CHANNEL, &adc_gpio_num );
    // assert( err == ESP_OK );
    // printf("ADC2 channel %d @ GPIO %d \n", ADC2_EXAMPLE_CHANNEL, adc_gpio_num);

    //be sure to do the init before using adc2. 
    printf("adc2_init...\n");
    ESP_ERROR_CHECK(adc2_config_channel_atten(channel, atten));
}

uint32_t readAnalogADC1(adc1_channel_t channel){
    uint32_t adc_reading = 0;
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
            adc_reading += adc1_get_raw(channel);
        }
        adc_reading /= NO_OF_SAMPLES;

    return adc_reading;
}

uint32_t readSingleAnalogADC2(adc2_channel_t channel){
    esp_err_t err;
    int read_raw, count = 0;
    do {
        err = adc2_get_raw(channel, width, &read_raw);
        count++;
    } while (err != ESP_OK && count < MAX_WAIT_ADC_2);
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
    return read_raw;
}

uint32_t readAnalogADC2(adc2_channel_t channel){
    uint32_t adc_reading = 0;
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
            adc_reading += readSingleAnalogADC2(channel);
        }
        adc_reading /= NO_OF_SAMPLES;

    return adc_reading;
}