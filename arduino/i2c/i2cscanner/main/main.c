/**
 * Scan for I2C devices.
 *
 * Scan for I2C devices using the Arduino Wire library.
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
#include "driver/gpio.h"
#include "esp_log.h"

extern void I2CScanner();

esp_err_t event_handler(void *ctx, system_event_t *event)
{
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
    I2CScanner();
    return 0;
}

