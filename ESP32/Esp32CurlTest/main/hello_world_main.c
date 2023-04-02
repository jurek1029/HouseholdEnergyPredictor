/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "espcurl.h"

static uint8_t s_led_state = 0;


static void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(33, s_led_state);
}

static void configure_led(void)
{
    ESP_LOGI("Test:", "Example configured to blink GPIO LED!");
    gpio_reset_pin(33);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(33, GPIO_MODE_OUTPUT);
}

void app_main(void)
{

    /* Configure the peripheral according to the LED type */
    configure_led();

    while (1) {
        ESP_LOGI("Test:", "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
        blink_led();
        /* Toggle the LED state */
        s_led_state = !s_led_state;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void print_response(char *hd, char * bdy, int res) {
	if (res) {
		printf("     ERROR: %d [%s]\r\n", res, bdy);
	}
	else {
		if (print_header) {
			printf("\r\n____________ Response HEADER: ____________\r\n%s\r\n^^^^^^^^^^^^ Response HEADER: ^^^^^^^^^^^^\r\n", hd);
		}
		if (print_body) {
			printf("\r\n____________ Response BODY: ____________\r\n%s\r\n^^^^^^^^^^^^ Response BODY: ^^^^^^^^^^^^\r\n", bdy);
		}
		if ((!print_header) && (!print_body)) printf("     OK.");
	}
	last_error = res;
	if (res) num_errors++;
	else num_errors = 0;
}

static void testGET()
{
	int res = 0;
    char *hdrbuf = calloc(1024, 1);
    assert(hdrbuf);
    char *bodybuf = calloc(4096, 1);
    assert(bodybuf);

    printf("\r\n\r\n#### HTTP GET\r\n");
    printf("     Send GET request with parameters\r\n");

    res = Curl_GET(Get_testURL, NULL, hdrbuf, bodybuf, 1024, 4096);
    print_response(hdrbuf, bodybuf,res);
	if (res) goto exit;
    vTaskDelay(2000 / portTICK_RATE_MS);

	printf("\r\n\r\n#### HTTP GET FILE\r\n");
	printf("     Get file (~225 KB) from http server");

	int exists = check_file("/spiflash/tiger.bmp");
	if (exists == 0) {
		printf(" and save it to file system\r\n");
		res = Curl_GET(Get_file_testURL, "/spiflash/tiger.bmp", hdrbuf, bodybuf, 1024, 4096);
	}
	else {
		printf(", simulate saving to file system\r\n");
		res = Curl_GET(Get_file_testURL, SIMULATE_FS, hdrbuf, bodybuf, 1024, 4096);
	}

	print_response(hdrbuf, bodybuf,res);
	if (res) goto exit;
    vTaskDelay(1000 / portTICK_RATE_MS);

	printf("\r\n\r\n#### HTTP GET BIG FILE\r\n");
	printf("     Get big file (~2.4 MB), simulate saving to file system\r\n");

	uint16_t tmo = curl_timeout;
	uint32_t maxb = curl_maxbytes;

	curl_timeout = 90;
	curl_maxbytes = 3000000;

	res = Curl_GET(Get_bigfile_testURL, SIMULATE_FS, hdrbuf, bodybuf, 1024, 4096);
	print_response(hdrbuf, bodybuf,res);

	curl_timeout = tmo;
	curl_maxbytes = maxb;

exit:
    free(bodybuf);
    free(hdrbuf);
    vTaskDelay(1000 / portTICK_RATE_MS);
}




esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
	if (_restarting) return ESP_OK;

    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
		ESP_LOGI(tag, "SYSTEM_EVENT_STA_START");
		ESP_ERROR_CHECK(esp_wifi_connect());
		break;
    case SYSTEM_EVENT_STA_GOT_IP:
		ESP_LOGI(tag, "SYSTEM_EVENT_STA_GOT_IP");
		ESP_LOGI(tag, "got ip:%s ... ready to go!\n", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
		if (thread_started == 0) {
			xTaskCreatePinnedToCore(&testCurl, "testCurl", 10*1024, NULL, 5, NULL, tskNO_AFFINITY);
			thread_started = 1;
		}
		break;
    case SYSTEM_EVENT_STA_CONNECTED:
		ESP_LOGI(tag, "SYSTEM_EVENT_STA_CONNECTED");
		break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
		ESP_LOGI(tag, "SYSTEM_EVENT_STA_DISCONNECTED");
		ESP_ERROR_CHECK(esp_wifi_connect());
		break;
    default:
		ESP_LOGI(tag, "=== WiFi EVENT: %d ===", event->event_id);
        break;
    }
    return ESP_OK;

}

//--------------------
static void mount_fs()
{
    ESP_LOGI(tag, "Mounting FAT filesystem");
    // To mount device we need name of device partition, define base_path
    // and allow format partition in case if it is new one and was not formated before
    const esp_vfs_fat_mount_config_t mount_config = {
            .max_files = 4,
            .format_if_mount_failed = true
    };
    esp_err_t err = esp_vfs_fat_spiflash_mount(base_path, "storage", &mount_config, &s_wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(tag, "=====================");
        ESP_LOGE(tag, "Failed to mount FATFS (0x%x)", err);
        ESP_LOGE(tag, "=====================\r\n");
        return;
    }

    ESP_LOGI(tag, "==============");
    ESP_LOGI(tag, "FATFS mounted.");
    ESP_LOGI(tag, "==============\r\n");
    has_fs = 1;
}


//================
int app_main(void)
{
    // Initialize NVS
    nvs_ok = nvs_flash_init();
    if (nvs_ok == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        const esp_partition_t* nvs_partition = esp_partition_find_first(
                ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);
        assert(nvs_partition && "partition table must have an NVS partition");
        ESP_ERROR_CHECK( esp_partition_erase_range(nvs_partition, 0, nvs_partition->size) );
        // Retry nvs_flash_init
        nvs_ok = nvs_flash_init();
    }
    if (nvs_ok == ESP_OK) {
		// Open NVS
		nvs_handle my_handle;
		esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
		if (err == ESP_OK) {
			err = nvs_get_u32(my_handle, "restart_cnt", &restart_cnt);
			if (err != ESP_OK) {
				restart_cnt = 0;
				err = nvs_set_i32(my_handle, "restart_cnt", restart_cnt);
				err = nvs_commit(my_handle);
				nvs_close(my_handle);
			}
		}
    }
	mount_fs();
	vTaskDelay(1000 / portTICK_RATE_MS);

	tcpip_adapter_init();

#ifndef USE_GSM
	ESP_ERROR_CHECK( esp_event_loop_init(wifi_event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    wifi_config_t sta_config = {
        .sta = {
            .ssid = SSID,
            .password = PASSWORD,
            .bssid_set = 0
        }
    };
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_connect() );
    ESP_ERROR_CHECK( esp_wifi_set_ps(WIFI_PS_NONE) );
#else
	if (doPPPoS_Connect() == 0) {
		ESP_LOGE(tag, "ERROR: GSM not initialized.");
		while (1) {
			vTaskDelay(1000 / portTICK_RATE_MS);
		}
	}

	if (thread_started == 0) {
		xTaskCreatePinnedToCore(&testCurl, "testCurl", 10*1024, NULL, 5, NULL, tskNO_AFFINITY);
		thread_started = 1;
	}
#endif

    return 0;
}
