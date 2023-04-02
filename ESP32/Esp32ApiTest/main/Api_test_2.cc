#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "protocol_examples_common.h"
#include "esp_sntp.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

//#include "time_sync.h"

#include "solcast_api.h"
#include "http_request.h"

const char capacity[] = "5";
const char tilt[] = "42";
const char azimuth[] = "180";
const char hours[] = "48";
const char lat[] = "-33.856784";
const char lon[] = "151.215297";

extern "C"{
    void app_main(void)
    {
        ESP_ERROR_CHECK(nvs_flash_init());
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());

        ESP_ERROR_CHECK(example_connect());

        request::setupHttps();
        std::unique_ptr<solcast::Data> pData = solcast::requestForecastData(
            solcast::SolcastApiData(lat,lon,capacity,tilt,azimuth,hours));

        printf("power: \n");
        int i = 0;
        for(auto v : pData->power){
            if(++i % 10 == 0) printf("\n");
            printf("%f,",v);
        }

        printf("\n time: \n");
        i=0;
        for(auto v : pData->time){
            if(++i % 10 == 0) printf("\n");
            printf("%s,",v.c_str());
        }

        printf("\npower size: %d \n",pData->time.size());
        printf("time size: %d \n", pData->power.size());
        printf("period: %f\n", pData->period);

    }
}