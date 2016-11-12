/**
 * Test the telnet functions.
 *
 * Perform a test using the telnet functions.
 *
 * When we run this sample, it will listen on port 23 which is the
 * default telnet port.  We can then connect with a telnet client.
 * Any data we receive will be consumed and a thank you message
 * sent.
 *
 * For additional details and documentation see:
 * * Free book on ESP32 - https://leanpub.com/kolban-ESP32
 *
 *
 * Neil Kolban <kolban1@kolban.com>
 *
 */
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "string.h"
#include "telnet.h"
#include "sdkconfig.h"

// YOUR network SSID
#define SSID "RASPI3"

// YOUR network password
#define PASSWORD "password"

static char tag[] = "telnet";

static void recvData(uint8_t *buffer, size_t size) {
	char responseMessage[100];
	ESP_LOGD(tag, "We received: %.*s", size, buffer);
	sprintf(responseMessage, "Thanks for %d bytes of data\n", size);
	telnet_esp32_sendData((uint8_t *)responseMessage, strlen(responseMessage));
}

static void telnetTask(void *data) {
	ESP_LOGD(tag, ">> telnetTask");
	telnet_esp32_listenForClients(recvData);
	ESP_LOGD(tag, "<< telnetTask");
	vTaskDelete(NULL);
}


static esp_err_t event_handler(void *ctx, system_event_t *event)
{
	if (event->event_id == SYSTEM_EVENT_STA_GOT_IP) {
		xTaskCreatePinnedToCore(&telnetTask, "telnetTask", 8048, NULL, 5, NULL, 0);
	}
  return ESP_OK;
}


int app_main(void)
{
    nvs_flash_init();
    system_init();
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
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

    return 0;
} // End of app_main

