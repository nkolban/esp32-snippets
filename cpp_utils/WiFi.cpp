/*
 * WiFi.cpp
 *
 *  Created on: Feb 25, 2017
 *      Author: kolban
 */

#include "WiFi.h"
#include <esp_event.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <nvs_flash.h>

#include "sdkconfig.h"

#include <lwip/sockets.h>
#include <string>
#include <string.h>

//static char tag[]= "WiFi";

WiFi::WiFi() {
	ip      = "192.168.1.99";
	gw      = "192.168.1.1";
	netmask = "255.255.255.0";
	wifiEventHandler = new WiFiEventHandler();
}


/**
 * Connect to an access point.
 * @param[in] ssid The network SSID of the access point to which we wish to connect.
 * @param[in] password The password of the access point to which we wish to connect.
 */
void WiFi::connectAP(std::string ssid, std::string password){
	::nvs_flash_init();
	::tcpip_adapter_init();

	::tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA); // Don't run a DHCP client
	tcpip_adapter_ip_info_t ipInfo;

	inet_pton(AF_INET, ip.data(), &ipInfo.ip);
	inet_pton(AF_INET, gw.data(), &ipInfo.gw);
	inet_pton(AF_INET, netmask.data(), &ipInfo.netmask);
	::tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);


	ESP_ERROR_CHECK( esp_event_loop_init(wifiEventHandler->getEventHandler(), wifiEventHandler));
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(::esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(::esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(::esp_wifi_set_mode(WIFI_MODE_STA));
	wifi_config_t sta_config;
	::memset(&sta_config, 0, sizeof(sta_config));
	::memcpy(sta_config.sta.ssid, ssid.data(), ssid.size());
	::memcpy(sta_config.sta.password, password.data(), password.size());
	sta_config.sta.bssid_set = 0;
	ESP_ERROR_CHECK(::esp_wifi_set_config(WIFI_IF_STA, &sta_config));
	ESP_ERROR_CHECK(::esp_wifi_start());
	ESP_ERROR_CHECK(::esp_wifi_connect());
} // connectAP


/**
 * Start being an access point.
 * @param[in] ssid The SSID to use to advertize for stations.
 * @param[in] password The password to use for station connections.
 */
void WiFi::startAP(std::string ssid, std::string password) {
	::nvs_flash_init();
	::tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_init(wifiEventHandler->getEventHandler(), wifiEventHandler));
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
	ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) );
	wifi_config_t apConfig;
	::memset(&apConfig, 0, sizeof(apConfig));
	::memcpy(apConfig.ap.ssid, ssid.data(), ssid.size());
	apConfig.ap.ssid_len = 0;
	::memcpy(apConfig.ap.password, password.data(), password.size());
	apConfig.ap.channel = 0;
	apConfig.ap.authmode = WIFI_AUTH_OPEN;
	apConfig.ap.ssid_hidden = 0;
	apConfig.ap.max_connection = 4;
	apConfig.ap.beacon_interval = 100;
	ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_AP, &apConfig) );
	ESP_ERROR_CHECK( esp_wifi_start() );
} // startAP
