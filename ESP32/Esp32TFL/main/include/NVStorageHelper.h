#ifndef NV_STORAGE_HELPER_H
#define NV_STORAGE_HELPER_H

#include "esp_system.h"

namespace NVStorageHelper{
     esp_err_t loadValuesFromNVS(const char* name, void* value);
     esp_err_t saveValuesToNVS(const char* name, size_t required_size, void* value);
}
#endif  // NV_STORAGE_HELPER_H