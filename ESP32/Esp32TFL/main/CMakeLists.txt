
#
# Main component of TF Micro project 'micro_speech'.
#

idf_component_register(
    SRCS main.cc main_functions.cc NTPTime.cc ADCHelper.cc LoadPrediction.cc model.cc ExponentialSmoothing.cc
    #PRIV_REQUIRES esp32-DHT11 esp_adc_cal nvs_flash tflite-lib spi_flash  
    INCLUDE_DIRS "")
