#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "curl/curl.h"

#define SSID "RASPI3"
#define PASSWORD "password"

/*
#define SSID "guest"
#define PASSWORD "kolbanguest"
*/

static char tag[] = "main";

static size_t writeData(void *buffer, size_t size, size_t nmemb, void *userp) {
	ESP_LOGI(tag, "writeData called: buffer=0x%lx, size=%d, nmemb=%d", (unsigned long)buffer, size, nmemb);
	ESP_LOGI(tag, "data>> %.*s", size*nmemb, (char *)buffer);
	return size * nmemb;
}

void testCurl(void *taskData) {
	CURL *handle;
	CURLcode res;

	curl_version_info_data *data = curl_version_info(CURLVERSION_NOW);
	ESP_LOGI(tag, "Curl version info");
	ESP_LOGI(tag, "version: %s - %d", data->version, data->version_num);
	ESP_LOGI(tag, "host: %s", data->host);
	if (data->features & CURL_VERSION_IPV6) {
		ESP_LOGI(tag, "- IP V6 supported");
	} else {
		ESP_LOGI(tag, "- IP V6 NOT supported");
	}
	if (data->features & CURL_VERSION_SSL) {
		ESP_LOGI(tag, "- SSL supported");
	} else {
		ESP_LOGI(tag, "- SSL NOT supported");
	}
	if (data->features & CURL_VERSION_LIBZ) {
		ESP_LOGI(tag, "- LIBZ supported");
	} else {
		ESP_LOGI(tag, "- LIBZ NOT supported");
	}
	if (data->features & CURL_VERSION_NTLM) {
		ESP_LOGI(tag, "- NTLM supported");
	} else {
		ESP_LOGI(tag, "- NTLM NOT supported");
	}
	if (data->features & CURL_VERSION_DEBUG) {
		ESP_LOGI(tag, "- DEBUG supported");
	} else {
		ESP_LOGI(tag, "- DEBUG NOT supported");
	}
	if (data->features & CURL_VERSION_UNIX_SOCKETS) {
		ESP_LOGI(tag, "- UNIX sockets supported");
	} else {
		ESP_LOGI(tag, "- UNIX sockets NOT supported");
	}
	ESP_LOGI(tag, "Protocols:");
	int i=0;
	while(data->protocols[i] != NULL) {
		ESP_LOGI(tag, "- %s", data->protocols[i]);
		i++;
	}



	// Create a curl handle
	handle = curl_easy_init();
	if (handle == NULL) {
		ESP_LOGI(tag, "Failed to create a curl handle");
		vTaskDelete(NULL);
		return;
	}

	ESP_LOGI(tag, "Created a curl handle ...");

	//char *url = "http://example.com";
	char *url = "ftp://pi:raspberry@192.168.5.1/.profile";
  curl_easy_setopt(handle, CURLOPT_URL, url);
  /* example.com is redirected, so we tell libcurl to follow redirection */
  curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeData);
  /* Perform the request, res will get the return code */
  res = curl_easy_perform(handle);
  if (res != CURLE_OK) {
  	ESP_LOGI(tag, "curl_easy_perform failed: %s", curl_easy_strerror(res));
  } else {
  	ESP_LOGI(tag, "curl_easy_perform() completed without incident.");
  }
  curl_easy_cleanup(handle);
	vTaskDelete(NULL);
} // End of testCurl


esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
	if (event->event_id == SYSTEM_EVENT_STA_GOT_IP) {
		ESP_LOGI(tag, "Got an IP ... ready to go!");
		xTaskCreatePinnedToCore(&testCurl, "testCurl", 10000, NULL, 5, NULL, 0);
		return ESP_OK;
	}
  return ESP_OK;
}

int app_main(void)
{
    nvs_flash_init();
    system_init();
    tcpip_adapter_init();
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
    ESP_LOGI(tag, "Registering test vfs");
    return 0;
}

