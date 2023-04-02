#include "NTPTime.h"

// #include <time.h>
// #include <sys/time.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

// #include "esp_system.h"
// #include "esp_event.h"

// #include "nvs_flash.h"
// #include "protocol_examples_common.h"
// #include "esp_sntp.h"
// #include "WiFi.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_sntp.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

static const char *TAG = "time_sync";
#define TIME_PERIOD (86400000000ULL)

#define STORAGE_NAMESPACE "storage"


bool NTPTime::isInitialized = false;

void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}

static void obtain_time(void)
{
#ifdef LWIP_DHCP_GET_NTP_SRV
    sntp_servermode_dhcp(1);
#endif

    // wait for time to be set
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void fetch_and_store_time_in_nvs(void *args)
{
    initialize_sntp();
    obtain_time();

    nvs_handle_t my_handle;
    esp_err_t err;

    time_t now;
    time(&now);

    //Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error updating time in nvs");

    //Write
    err = nvs_set_i64(my_handle, "timestamp", now);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error updating time in nvs");

    err = nvs_commit(my_handle);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error updating time in nvs");

    nvs_close(my_handle);
    sntp_stop();

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error updating time in nvs");
    } else {
        ESP_LOGI(TAG, "Updated time in NVS");
    }
}

esp_err_t update_time_from_nvs(void)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS");
        nvs_close(my_handle);
        return err;
    }

    int64_t timestamp = 0;

    err = nvs_get_i64(my_handle, "timestamp", &timestamp);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        fetch_and_store_time_in_nvs(NULL);
        err = ESP_OK;
    } else if (err == ESP_OK) {
        struct timeval get_nvs_time;
        get_nvs_time.tv_sec = timestamp;
        settimeofday(&get_nvs_time, NULL);
    }   

    nvs_close(my_handle);
    return err;
}

void NTPTime::setupNTPTime(){
    if(!NTPTime::isInitialized){
        if (esp_reset_reason() == ESP_RST_POWERON) {
            ESP_ERROR_CHECK(update_time_from_nvs());
        }

        const esp_timer_create_args_t nvs_update_timer_args = {
                .callback = &fetch_and_store_time_in_nvs,
        };

        esp_timer_handle_t nvs_update_timer;
        ESP_ERROR_CHECK(esp_timer_create(&nvs_update_timer_args, &nvs_update_timer));
        ESP_ERROR_CHECK(esp_timer_start_periodic(nvs_update_timer, TIME_PERIOD));

        setenv("TZ", NTPTime::TIME_ZONE, 1);
        tzset();
        NTPTime::isInitialized = true;
    }
}

