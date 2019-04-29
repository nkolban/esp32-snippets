/*
 * BootWiFi.h
 *
 *  Created on: Nov 25, 2016
 *      Author: kolban
 */

#ifndef MAIN_BOOTWIFI_H_
#define MAIN_BOOTWIFI_H_

#include <WiFi.h>
#include <HttpServer.h>
#include <FreeRTOS.h>

#define RESTART_COUNTER
#define MAX_RESTART_TO_CONFIG (3)

typedef void (*bootwifi_callback_t)(int rc);
class BootWifiEventHandler;

class BootWiFi {
private:
	friend BootWifiEventHandler;
	void bootWiFi2();
	WiFi        m_wifi;
	HttpServer  m_httpServer;
	bool        m_httpServerStarted;
	std::string m_ssid;
	std::string m_password;
  uint8_t     m_apConnectionStatus;   // receives the connection status.  ESP_OK = received SYSTEM_EVENT_STA_GOT_IP event.
	FreeRTOS::Semaphore m_completeSemaphore = FreeRTOS::Semaphore("completeSemaphore");

public:
	BootWiFi();
	BootWiFi(char *);
	void setAccessPointCredentials(std::string ssid, std::string password);
	uint8_t boot();
	uint8_t boot(std::string);
	std::string getIp(void);
};

#endif /* MAIN_BOOTWIFI_H_ */
