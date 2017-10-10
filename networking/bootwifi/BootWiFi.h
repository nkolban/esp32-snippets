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

typedef void (*bootwifi_callback_t)(int rc);
class BootWifiEventHandler;

class BootWiFi {
private:
	friend BootWifiEventHandler;
	void bootWiFi2();
	WiFi       m_wifi;
	HttpServer m_httpServer;
	bool       m_httpServerStarted;
	FreeRTOS::Semaphore m_completeSemaphore = FreeRTOS::Semaphore("completeSemaphore");

public:
	BootWiFi();
	void boot();
};

#endif /* MAIN_BOOTWIFI_H_ */
