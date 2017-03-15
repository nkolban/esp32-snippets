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
#include <lwip/dns.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <string>
#include <string.h>
#include <Task.h>

#include "sdkconfig.h"

static char tag[]= "WiFi";


/*
static void setDNSServer(char *ip) {
	ip_addr_t dnsserver;
	ESP_LOGD(tag, "Setting DNS[%d] to %s", 0, ip);
	inet_pton(AF_INET, ip, &dnsserver);
	ESP_LOGD(tag, "ip of DNS is %.8x", *(uint32_t *)&dnsserver);
	dns_setserver(0, &dnsserver);
}
*/


WiFi::WiFi() {
	ip      = "";
	gw      = "";
	netmask = "";
	wifiEventHandler = new WiFiEventHandler();
}


/**
 * @brief Add a reference to a DNS server.
 *
 * Here we define a server that will act as a DNS server.  We can add two DNS
 * servers in total.  The first will be the primary, the second will be the backup.
 * The public Google DNS servers are "8.8.8.8" and "8.8.4.4".
 *
 * For example:
 *
 * @code{.cpp}
 * wifi.addDNSServer("8.8.8.8");
 * wifi.addDNSServer("8.8.4.4");
 * @endcode
 *
 * @param [in] ip The IP address of the DNS Server.
 */
void WiFi::addDNSServer(std::string ip) {
	ip_addr_t dnsserver;
	ESP_LOGD(tag, "Setting DNS[%d] to %s", dnsCount, ip.c_str());
	inet_pton(AF_INET, ip.c_str(), &dnsserver);
	::dns_setserver(dnsCount, &dnsserver);
	dnsCount++;
} // addDNSServer

/**
 * Connect to an access point.
 * @param[in] ssid The network SSID of the access point to which we wish to connect.
 * @param[in] password The password of the access point to which we wish to connect.
 */
void WiFi::connectAP(std::string ssid, std::string password){
	::nvs_flash_init();
	::tcpip_adapter_init();
	if (ip.length() > 0 && gw.length() > 0 && netmask.length() > 0) {
		::tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA); // Don't run a DHCP client
		tcpip_adapter_ip_info_t ipInfo;

		inet_pton(AF_INET, ip.data(), &ipInfo.ip);
		inet_pton(AF_INET, gw.data(), &ipInfo.gw);
		inet_pton(AF_INET, netmask.data(), &ipInfo.netmask);
		::tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
	}


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
 * @brief Dump diagnostics to the log.
 */
void WiFi::dump() {
	ESP_LOGD(tag, "WiFi Dump");
	ESP_LOGD(tag, "---------");
	char ipAddrStr[30];
	ip_addr_t ip = ::dns_getserver(0);
	inet_ntop(AF_INET, &ip, ipAddrStr, sizeof(ipAddrStr));
	ESP_LOGD(tag, "DNS Server[0]: %s", ipAddrStr);
} // dump


/**
 * @brief Lookup an IP address by host name.
 *
 * @param [in] hostname The hostname to resolve.
 *
 * @return The IP address of the host or 0.0.0.0 if not found.
 */
struct in_addr WiFi::getHostByName(std::string hostName) {
	struct in_addr retAddr;
	struct hostent *he = gethostbyname(hostName.c_str());
	if (he == nullptr) {
		retAddr.s_addr = 0;
		ESP_LOGD(tag, "Unable to resolve %s - %d", hostName.c_str(), h_errno);
	} else {
		retAddr = *(struct in_addr *)(he->h_addr_list[0]);
		//ESP_LOGD(tag, "resolved %s to %.8x", hostName, *(uint32_t *)&retAddr);

	}
	return retAddr;
} // getHostByName


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


/**
 * @brief Set the IP info used when connecting as a station to an access point.
 *
 * For example, prior to calling connectAP() we could invoke:
 *
 * @code{.cpp}
 * myWifi.setIPInfo("192.168.1.99", "192.168.1.1", "255.255.255.0");
 * @encode
 *
 * @param [in] ip IP address value.
 * @param [in] gw Gateway value.
 * @param [in] netmask Netmask value.
 */
void WiFi::setIPInfo(std::string ip, std::string gw, std::string netmask) {
	this->ip = ip;
	this->gw = gw;
	this->netmask = netmask;
}
