#include <esp_event.h>
#include <esp_event_loop.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <lwip/sockets.h>
#include <nvs_flash.h>

// The IP address that we want our device to have.
#define DEVICE_IP          "192.168.1.99"

// The Gateway address where we wish to send packets.
// This will commonly be our access point.
#define DEVICE_GW          "192.168.1.1"

// The netmask specification.
#define DEVICE_NETMASK     "255.255.255.0"

void task_paho(void *ignore);


esp_err_t esp32_wifi_eventHandler(void *ctx, system_event_t *event) {
	// Your event handling code here...
	if (event->event_id == SYSTEM_EVENT_STA_GOT_IP) {
    xTaskCreatePinnedToCore(&task_paho, "task_paho", 8048, NULL, 5, NULL, 0);
	}
	return ESP_OK;
}

void app_main(void)
{
    nvs_flash_init();
    tcpip_adapter_init();

    tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA); // Don't run a DHCP client
    tcpip_adapter_ip_info_t ipInfo;

    inet_pton(AF_INET, DEVICE_IP, &ipInfo.ip);
    inet_pton(AF_INET, DEVICE_GW, &ipInfo.gw);
    inet_pton(AF_INET, DEVICE_NETMASK, &ipInfo.netmask);
    tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);

    ESP_ERROR_CHECK( esp_event_loop_init(esp32_wifi_eventHandler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    wifi_config_t sta_config = {
        .sta = {
            .ssid = "<Your SSID>",
            .password = "<Your Password>",
            .bssid_set = false
        }
    };
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_connect() );
}
