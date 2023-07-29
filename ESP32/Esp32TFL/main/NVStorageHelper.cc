#include"NVStorageHelper.h"

#include <stdio.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define STORAGE_NAMESPACE "storage"

namespace NVStorageHelper{

    esp_err_t loadValuesFromNVS(const char* name, void* value){  
        nvs_handle_t  handle;
        esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
        if (err != ESP_OK) return err;

        size_t required_size = 0; 
        err = nvs_get_blob(handle, name, NULL, &required_size);
        if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;

        if (required_size > 0) {
            printf("req size: %s", name);
            err = nvs_get_blob(handle, name, value, &required_size);
            if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND){
                return err;  
            } 
        }
        nvs_close(handle);
        return ESP_OK;
    }

    esp_err_t saveValuesToNVS(const char* name, size_t required_size, void* value){
        nvs_handle_t  handle;
        esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
        if (err != ESP_OK) return err;
        
        err = nvs_set_blob(handle, name, value, required_size);
        if (err != ESP_OK) return err;

        // Commit
        err = nvs_commit(handle);
        if (err != ESP_OK) return err;

        // Close
        nvs_close(handle);
        return ESP_OK;
    }
}