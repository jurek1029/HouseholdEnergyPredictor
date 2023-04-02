/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "sdkconfig.h"

#include <string.h>
#include <stdlib.h>
#include "freertos/event_groups.h"
#include "esp_netif.h"

#include "esp_tls.h"
#include "esp_crt_bundle.h"

#define MAX_URLS    2
static const char *web_urls[MAX_URLS] = {
    "https://api.solcast.com.au",
    "https://espressif.com",
};

static const char *TAG = "example";

static void https_get_task(void *pvParameters)
{
    while (1) {
        int conn_count = 0;
        ESP_LOGI(TAG, "Connecting to %d URLs", MAX_URLS);

        for (int i = 0; i < MAX_URLS; i++) {
            esp_tls_cfg_t cfg = {
                .crt_bundle_attach = esp_crt_bundle_attach,
            };

            esp_tls_t *tls = esp_tls_init();
            if (!tls) {
                ESP_LOGE(TAG, "Failed to allocate esp_tls handle!");
                goto end;
            }

            if (esp_tls_conn_http_new_async(web_urls[i], &cfg, tls) == 1) {
                ESP_LOGI(TAG, "Connection established to %s", web_urls[i]);
                conn_count++;
            } else {
                ESP_LOGE(TAG, "Could not connect to %s", web_urls[i]);
            }

            esp_tls_conn_destroy(tls);
end:
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

        ESP_LOGI(TAG, "Completed %d connections", conn_count);
        ESP_LOGI(TAG, "Starting over again...");
    }
}

/*
// Constants that aren't configurable in menuconfig 
 #define WEB_SERVER "api.solcast.com.au"
//#define WEB_SERVER "google.com"
#define WEB_PORT "80"
//#define WEB_PATH "/world_radiation/forecasts?latitude=-33.856784&longitude=151.215297&output_parameters=air_temp,dni,ghi&format=json"
#define WEB_PATH "/"

static uint8_t s_led_state = 0;

static const char *TAG = "example";
// char recv_buf[20240];
char recv_buf[64];

static const char *REQUEST = "GET " WEB_PATH " HTTP/1.1\r\n"
    "Authorization: Bearer q_m_SaOSVkHui2bOMrGgzY6T68TbEpN7\r\n"
    "Host: "WEB_SERVER"\r\n"
    "Accept: text/html\r\n"
    // "User-Agent: esp-idf/1.0 esp32\r\n"
    //"User-Agent: Mozilla/5.0\r\n"
    //"Accept: application/json\r\n"
    "\r\n";
*/

/*
static void http_get_task(void *pvParameters)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    
    //char recv_buf[64];

//while(1){
    int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);

    if(err != 0 || res == NULL) {
        ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        //continue;
        return;
    }

    //  Code to print the resolved IP.

    //     Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code 
    addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
    ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

    s = socket(res->ai_family, res->ai_socktype, 0);
    if(s < 0) {
        ESP_LOGE(TAG, "... Failed to allocate socket.");
        freeaddrinfo(res);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        // continue;
        return;
    }
    ESP_LOGI(TAG, "... allocated socket");

    if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
        ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
        close(s);
        freeaddrinfo(res);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
        // continue;
        return;
    }

    ESP_LOGI(TAG, "... connected");
    freeaddrinfo(res);

    if (write(s, REQUEST, strlen(REQUEST)) < 0) {
        ESP_LOGE(TAG, "... socket send failed");
        close(s);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
        // continue;
        return;
    }
    ESP_LOGI(TAG, "... socket send success");

    struct timeval receiving_timeout;
    receiving_timeout.tv_sec = 5;
    receiving_timeout.tv_usec = 0;
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
            sizeof(receiving_timeout)) < 0) {
        ESP_LOGE(TAG, "... failed to set socket receiving timeout");
        close(s);
        vTaskDelay(4000 / portTICK_PERIOD_MS);
        // continue;
        return;
    }
    ESP_LOGI(TAG, "... set socket receiving timeout success");

    // Read HTTP response 
    do {
        bzero(recv_buf, sizeof(recv_buf));
        r = read(s, recv_buf, sizeof(recv_buf)-1);
        for(int i = 0; i < r; i++) {
            putchar(recv_buf[i]);
        }
    } while(r > 0);

    ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d.", r, errno);
    close(s);
    for(int countdown = 10; countdown >= 0; countdown--) {
        ESP_LOGI(TAG, "%d... ", countdown);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
*/

void app_main(void)
{

    ESP_ERROR_CHECK( nvs_flash_init() );
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    /* Configure the peripheral according to the LED type */

    ESP_ERROR_CHECK(example_connect());

  //  ESP_LOGI("Test:", "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
    //http_get_task(NULL);
    //xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);
    xTaskCreate(&https_get_task, "https_get_task", 8192, NULL, 5, NULL);
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
