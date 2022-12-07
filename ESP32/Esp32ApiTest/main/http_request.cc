#include <sstream>
#include "esp_tls.h"
#include "esp_log.h"
#include "http_request.h"
#include "time_sync.h"

#define TIME_PERIOD (86400000000ULL)

namespace request{
    const char *TAG = "http_request";

    void setupHttps(){
            if (esp_reset_reason() == ESP_RST_POWERON) {
                ESP_ERROR_CHECK(update_time_from_nvs());
            }

            const esp_timer_create_args_t nvs_update_timer_args = {
                    .callback = &fetch_and_store_time_in_nvs,
            };

            esp_timer_handle_t nvs_update_timer;
            ESP_ERROR_CHECK(esp_timer_create(&nvs_update_timer_args, &nvs_update_timer));
            ESP_ERROR_CHECK(esp_timer_start_periodic(nvs_update_timer, TIME_PERIOD));

    }

    std::unique_ptr<std::stringstream> httpsGetRequest(UrlData &&urldata){
        esp_tls_cfg_t cfg = {
            .cacert_buf = (const unsigned char *) server_root_cert_pem_start,
            .cacert_bytes = (unsigned int) (server_root_cert_pem_end - server_root_cert_pem_start),
        };
        const char *WEB_SERVER_URL = urldata.url.get();
        const char *REQUEST = urldata.headers.get();

        char buf[512];
        int ret, len;
        // std::stringstream ss("",std::ios_base::app);
        std::unique_ptr<std::stringstream> ssOut(new std::stringstream(""));
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
            ESP_LOGD(TAG, "%d bytes read", len);
            #ifdef DEBUG_HTTP_REQUEST
            /* Print response directly to stdout as it is read */
            for (int i = 0; i < len; i++) {
                putchar(buf[i]);
            }
            putchar('\n'); // JSON output doesn't have a newline at end
            #endif
            *ssOut.get() << buf;
            
        } while (moredata);
        esp_tls_conn_destroy(tls);
        return ssOut;
    }
}