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
#include "WiFiEventHandler.h"



class WiFiAPRecord {
public:
	friend class WiFi;
	wifi_auth_mode_t getAuthMode() {
		return authMode;
	}

	int8_t getRSSI() {
		return rssi;
	}

	std::string getSSID() {
		return ssid;
	}

	std::string toString();
private:
	uint8_t bssid[6];
	int8_t rssi;
	std::string ssid;
	wifi_auth_mode_t authMode;
};
/**
 * @brief %WiFi driver.
 *
 * Encapsulate control of %WiFi functions.
 *
 * Here is an example fragment that illustrates connecting to an access point.
 * @code{.cpp}
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
 * MyWiFiEventHandler *eventHandler = new MyWiFiEventHandler();
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
	int dnsCount=0;
	char *dnsServer = nullptr;
};

#endif // CONFIG_WIFI_ENABLED
#endif /* MAIN_WIFI_H_ */
