
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <memory>
#include <map>
#include <sstream>
#include <vector>
//#include <bits/stdc++.h>
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

#include "esp_tls.h"
#include "sdkconfig.h"
#include "time_sync.h"

/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "api.solcast.com.au"
#define WEB_PORT "443"
#define WEB_URL "https://api.solcast.com.au/world_pv_power/forecasts?latitude=%s&longitude=%s&output_parameters=pv_power_rooftop&capacity=%s&tilt=%s&azimuth=%s&hours=%s&format=csv"
//#define WEB_URL "https://api.solcast.com.au/world_radiation/forecasts?latitude=-33.856784&longitude=151.215297&output_parameters=air_temp,dni,ghi"

#define SERVER_URL_MAX_SZ 256

static const char *TAG = "example";

/* Timer interval once every day (24 Hours) */
#define TIME_PERIOD (86400000000ULL)

static const char HEADERS[] = "Host: " WEB_SERVER "\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "Authorization: Bearer q_m_SaOSVkHui2bOMrGgzY6T68TbEpN7\r\n"
                             "\r\n";

const char capacity[] = "5";
const char tilt[] = "42";
const char azimuth[] = "180";
const char hours[] = "48";
const char lat[] = "-33.856784";
const char lon[] = "151.215297";
std::map<std::string,std::string> csvData;

/* 
   The PEM file was extracted from the output of this command:
   openssl s_client -showcerts -connect www.howsmyssl.com:443 </dev/null

   The CA root cert is the last cert given in the chain of certs.

   To embed it in the app binary, the PEM file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/
extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");
extern const uint8_t server_root_cert_pem_end[]   asm("_binary_server_root_cert_pem_end");

struct Data {
    std::vector<float> power;
    std::vector<std::string> time;
    float period;
}

struct UrlData {
    std::unique_ptr<char> url;
    std::unique_ptr<char> headers;
    std::unique_ptr<Data> out;
    UrlData() {};
    //UrlData(const char* _url, const char* _headers): url(_url), headers(_headers){};
    UrlData(std::unique_ptr<char> &_url, std::unique_ptr<char> &_headers, std::unique_ptr<Data> _out): url(move(_url)), headers(move(_headers)), out(move(_out)){};
};

struct SolcastApiData{
    const char* latitude;
    const char* longitude;
    const char* capacity;
    const char* tilt;
    const char* azimuth;
    const char* hours;
    SolcastApiData(const char* lat, const char* lon, const char* cap, const char* _tilt, const char* _azimuth, const char* _hours):
            latitude(lat), longitude(lon), capacity(cap), tilt(_tilt), azimuth(_azimuth), hours(_hours){};
};



static void https_get_request(esp_tls_cfg_t cfg, const char *WEB_SERVER_URL, const char *REQUEST, Data& out)
{
    char buf[512];
    int ret, len;
    // std::stringstream ss("",std::ios_base::app);
    std::stringstream ss("");
    bool moredata = true;

    esp_tls_t *tls = esp_tls_init();
    if (!tls) {
        ESP_LOGE(TAG, "Failed to allocate esp_tls handle!");
        for (int countdown = 10; countdown >= 0; countdown--) {
            ESP_LOGI(TAG, "%d...", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
    if (esp_tls_conn_http_new_async(WEB_SERVER_URL, &cfg, tls) == 1) {
        ESP_LOGI(TAG, "Connection established...");
    } else {
        ESP_LOGE(TAG, "Connection failed...");
        esp_tls_conn_destroy(tls);
        for (int countdown = 10; countdown >= 0; countdown--) {
            ESP_LOGI(TAG, "%d...", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }

    size_t written_bytes = 0;
    do {
        ret = esp_tls_conn_write(tls,REQUEST + written_bytes, strlen(REQUEST) - written_bytes);
        if (ret >= 0) {
            ESP_LOGI(TAG, "%d bytes written", ret);
            written_bytes += ret;
        } else if (ret != ESP_TLS_ERR_SSL_WANT_READ  && ret != ESP_TLS_ERR_SSL_WANT_WRITE) {
            ESP_LOGE(TAG, "esp_tls_conn_write  returned: [0x%02X](%s)", ret, esp_err_to_name(ret));
            esp_tls_conn_destroy(tls);
            for (int countdown = 10; countdown >= 0; countdown--) {
                ESP_LOGI(TAG, "%d...", countdown);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
        }
    } while (written_bytes < strlen(REQUEST));

    ESP_LOGI(TAG, "Reading HTTP response...");
    do {
        len = sizeof(buf) - 1;
        memset(buf, 0x00, sizeof(buf));
        ret = esp_tls_conn_read(tls, (char *)buf, len);

        if (ret == ESP_TLS_ERR_SSL_WANT_WRITE  || ret == ESP_TLS_ERR_SSL_WANT_READ) {
            continue;
        } else if (ret < 0) {
            ESP_LOGE(TAG, "esp_tls_conn_read  returned [-0x%02X](%s)", -ret, esp_err_to_name(ret));
            break;
        } else if (ret == 0) {
            ESP_LOGI(TAG, "connection closed");
            break;
        }
        else if(ret < 511){
            ESP_LOGI(TAG, "connection closed");
            moredata = false;
        }

        len = ret;
        ESP_LOGI(TAG, "%d bytes read", len);
        /* Print response directly to stdout as it is read */
        // for (int i = 0; i < len; i++) {
        //     putchar(buf[i]);
        // }
        // putchar('\n'); // JSON output doesn't have a newline at end
        ss << buf;
        
    } while (moredata);

    bool dataStarted = false;
    std::string line = "";
    while(std::getline(ss, line)){
        printf("line: %s\n", line.c_str());
        if(dataStarted){ 
            std::stringstream lss(line);
            std::string tag;
            while(std::getline(lss,tag,',')){
                
                printf("tag: %s\n",tag.c_str());
            }
        }
        else if(line.find("pv_estimate") != std::string::npos){
            dataStarted = true;
        }

       
    }
    esp_tls_conn_destroy(tls);
}




static void https_request_task(void *urldata)
{
    ESP_LOGI(TAG, "Start https_request example");
    ESP_LOGI(TAG, "Minimum free heap size: %d bytes", esp_get_minimum_free_heap_size());

    esp_tls_cfg_t cfg = {
        .cacert_buf = (const unsigned char *) server_root_cert_pem_start,
        .cacert_bytes = (unsigned int) (server_root_cert_pem_end - server_root_cert_pem_start),
    };
    UrlData* url = (UrlData*)urldata;
    https_get_request(cfg, url->url.get(), url->headers.get(), url->out.get());
    delete url;
    vTaskDelete(NULL);
}

static void httpsRequest(std::unique_ptr<char> &url, std::unique_ptr<char> &header, std::unique_ptr<Data> &out){
    UrlData* urldata = new UrlData(url, header, out);
    xTaskCreate(&https_request_task, "https_get_task", 8192, urldata, 5, NULL);
}

static std::unique_ptr<Data> solcastApi(SolcastApiData &&data){
    std::unique_ptr<char> url = std::unique_ptr<char>(new char[200]);
    sprintf(url.get(), WEB_URL, data.latitude, data.longitude, data.capacity, data.tilt, data.azimuth, data.hours);
    std::unique_ptr<char> bufHeader = std::unique_ptr<char>(new char[strlen(url.get()) + strlen(HEADERS) + 16]);
    sprintf(bufHeader.get(), "GET %s HTTP/1.1\r\n%s", url.get(), HEADERS);
    std::unique_ptr<Data> out = std::unique_ptr<Data>(new Data());
    httpsRequest(url, bufHeader, out);
    // TODO No xTask
}

extern "C"{
    void app_main(void)
    {
        ESP_ERROR_CHECK(nvs_flash_init());
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());

        ESP_ERROR_CHECK(example_connect());

        if (esp_reset_reason() == ESP_RST_POWERON) {
            ESP_LOGI(TAG, "Updating time from NVS");
            ESP_ERROR_CHECK(update_time_from_nvs());
        }

        const esp_timer_create_args_t nvs_update_timer_args = {
                .callback = &fetch_and_store_time_in_nvs,
        };

        esp_timer_handle_t nvs_update_timer;
        ESP_ERROR_CHECK(esp_timer_create(&nvs_update_timer_args, &nvs_update_timer));
        ESP_ERROR_CHECK(esp_timer_start_periodic(nvs_update_timer, TIME_PERIOD));

        solcastApi(SolcastApiData(lat,lon,capacity,tilt,azimuth,hours));

    }
}
