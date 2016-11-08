#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_log.h"

extern void listenForNewClient();
static char tag[] = "main";
void telnetTask(void *data) {
	ESP_LOGD(tag, ">> telnetTask");
	listenForNewClient();
	vTaskDelete(NULL);
}

esp_err_t event_handler(void *ctx, system_event_t *event)
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
            .ssid = "RASPI3",
            .password = "password",
            .bssid_set = 0
        }
    };
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_connect() );


    return 0;
}

