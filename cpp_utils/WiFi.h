/*
 * WiFi.h
 *
 *  Created on: Feb 25, 2017
 *      Author: kolban
 */

#ifndef MAIN_WIFI_H_
#define MAIN_WIFI_H_
#include <string>
#include "WiFiEventHandler.h"
/**
 * @brief WiFi driver.
 * Encapsulate control of WiFi functions.
 *
 */
class WiFi {
private:
	std::string ip;
	std::string gw;
	std::string netmask;
	WiFiEventHandler *wifiEventHandler;

public:
	WiFi();
	void connectAP(std::string ssid, std::string passwd);
	void startAP(std::string ssid, std::string passwd);

	/**
	 * Set the event handler to use to process detected events.
	 * @param[in] wifiEventHandler The class that will be used to process events.
	 */
	void setWifiEventHandler(WiFiEventHandler *wifiEventHandler) {
		this->wifiEventHandler = wifiEventHandler;
	}
};

#endif /* MAIN_WIFI_H_ */
