/* LwIP SNTP example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
// #include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
// #include "esp_log.h"
// #include "esp_attr.h"
// #include "esp_sleep.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "esp_sntp.h"


static void obtain_time(void);

extern "C" void app_main(void)
{

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900)) {
        printf("Time is not set yet. Connecting to WiFi and getting time over NTP. \n");
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }

    setenv("TZ", "UTC-2", 1);
    tzset();
    char strftime_buf[64];

    while (1){
        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        printf("The current date/time is: %s \n", strftime_buf);

        if (sntp_get_sync_mode() == SNTP_SYNC_MODE_SMOOTH) {
            struct timeval outdelta;
            while (sntp_get_sync_status() == SNTP_SYNC_STATUS_IN_PROGRESS) {
                adjtime(NULL, &outdelta);
                printf("Waiting for adjusting time ... outdelta = %li sec: %li ms: %li us \n",
                            (long)outdelta.tv_sec,
                            outdelta.tv_usec/1000,
                            outdelta.tv_usec%1000);
                vTaskDelay(2000 / portTICK_PERIOD_MS);
            }
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    
}

static void obtain_time(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /**
     * NTP server address could be aquired via DHCP,
     * see LWIP_DHCP_GET_NTP_SRV menuconfig option
     */
#ifdef LWIP_DHCP_GET_NTP_SRV
    sntp_servermode_dhcp(1);
#endif

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    // wait for time to be set
    //time_t now = 0;
    //struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        printf("Waiting for system time to be set... (%d/%d)\n", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    //time(&now);
    //localtime_r(&now, &timeinfo);

    ESP_ERROR_CHECK( example_disconnect() );
}