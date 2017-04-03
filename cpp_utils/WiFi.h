/*
 * WiFi.h
 *
 *  Created on: Feb 25, 2017
 *      Author: kolban
 */

#ifndef MAIN_WIFI_H_
#define MAIN_WIFI_H_
#include "sdkconfig.h"
#if defined(CONFIG_WIFI_ENABLED)
#include <string>
#include <vector>
#include <mdns.h>
#include "WiFiEventHandler.h"

/**
 * @brief Manage mDNS server.
 */
class MDNS {
public:
	MDNS();
	~MDNS();
	void serviceAdd(std::string service, std::string proto, uint16_t port);
	void serviceInstanceSet(std::string service, std::string proto, std::string instance);
	void servicePortSet(std::string service, std::string proto, uint16_t port);
	void serviceRemove(std::string service, std::string proto);
	void setHostname(std::string hostname);
	void setInstance(std::string instance);
private:
	mdns_server_t *m_mdns_server = nullptr;
};

class WiFiAPRecord {
public:
	friend class WiFi;
	wifi_auth_mode_t getAuthMode() {
		return m_authMode;
	}

	int8_t getRSSI() {
		return m_rssi;
	}

	std::string getSSID() {
		return m_ssid;
	}

	std::string toString();
private:
	uint8_t m_bssid[6];
	int8_t m_rssi;
	std::string m_ssid;
	wifi_auth_mode_t m_authMode;
};
/**
 * @brief %WiFi driver.
 *
 * Encapsulate control of %WiFi functions.
 *
 * Here is an example fragment that illustrates connecting to an access point.
 * @code{.cpp}
 * #include <WiFi.h>
 * #include <WiFiEventHandler.h>
 *
 * class TargetWiFiEventHandler: public WiFiEventHandler {
 *    esp_err_t staGotIp(system_event_sta_got_ip_t event_sta_got_ip) {
 *       ESP_LOGD(tag, "MyWiFiEventHandler(Class): staGotIp");
 *       // Do something ...
 *       return ESP_OK;
 *    }
 * };
 *
 * WiFi wifi;
 *
 * TargetWiFiEventHandler *eventHandler = new TargetWiFiEventHandler();
 * wifi.setWifiEventHandler(eventHandler);
 * wifi.connectAP("myssid", "mypassword");
 * @endcode
 */
class WiFi {
private:
	std::string      ip;
	std::string      gw;
	std::string      netmask;
	WiFiEventHandler *wifiEventHandler;

public:
	WiFi();
	void addDNSServer(std::string ip);
	struct in_addr getHostByName(std::string hostName);
	void connectAP(std::string ssid, std::string passwd);
	void dump();
	std::vector<WiFiAPRecord> scan();
	void startAP(std::string ssid, std::string passwd);
	void setIPInfo(std::string ip, std::string gw, std::string netmask);



	/**
	 * Set the event handler to use to process detected events.
	 * @param[in] wifiEventHandler The class that will be used to process events.
	 */
	void setWifiEventHandler(WiFiEventHandler *wifiEventHandler) {
		this->wifiEventHandler = wifiEventHandler;
	}
private:
	int m_dnsCount=0;
	//char *m_dnsServer = nullptr;

};

#endif // CONFIG_WIFI_ENABLED
#endif /* MAIN_WIFI_H_ */
