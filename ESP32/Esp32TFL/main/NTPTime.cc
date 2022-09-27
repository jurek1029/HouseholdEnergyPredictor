#include "NTPTime.h"

#include <time.h>
#include <sys/time.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

// #include "esp_system.h"
// #include "esp_event.h"

// #include "nvs_flash.h"
// #include "protocol_examples_common.h"
#include "esp_sntp.h"
#include "WiFi.h"

void NTPTime::obtain_time()
{
    wifi::setupWiFi();
    wifi::connect();

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        printf("Waiting for system time to be set... (%d/%d)\n", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    wifi::disconnect();
}

void NTPTime::setupNTPTime(){
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2020 - 1900)) {
        printf("Time is not set yet. Connecting to WiFi and getting time over NTP. \n");
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }
    setenv("TZ", NTPTime::TIME_ZONE, 1);
    tzset();
}
