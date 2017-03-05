/*
 * WiFiEventHandler.h
 *
 *  Created on: Feb 25, 2017
 *      Author: kolban
 *
 * A WiFiEventHandler defines a class that has methods that will be called back when a WiFi event is
 * detected.
 *
 * Typically this class is subclassed to provide implementations for the callbacks we want to handle.
 *
 * class MyHandler: public WiFiEventHandler {
 *   esp_err_t apStart() {
 *      ESP_LOGD(tag, "MyHandler(Class): apStart");
 *      return ESP_OK;
 *   }
 * }
 *
 * WiFi wifi;
 * MyHandler *eventHandler = new MyHandler();
 * wifi.setWifiEventHandler(eventHandler);
 * wifi.startAP("MYSSID", "password");
 *
 * The overridable functions are:
 * * esp_err_t apStaConnected()
 * * esp_err_t apStaDisconnected()
 * * esp_err_t apStart()
 * * esp_err_t apStop()
 * * esp_err_t staConnected()
 * * esp_err_t staDisconnected()
 * * esp_err_t staGotIp(system_event_sta_got_ip_t event_sta_got_ip)
 * * esp_err_t staStart()
 * * esp_err_t staStop()
 * * esp_err_t wifiReady()
 */

#ifndef MAIN_WIFIEVENTHANDLER_H_
#define MAIN_WIFIEVENTHANDLER_H_
#include <esp_event.h>
#include <esp_event_loop.h>

/**
 * @brief WiFi state event handler.
 *
 * Here is an example class that implements all the virtual functions that can be called
 * for events:
 *
 * @code{.cpp}
 * MyHandler::MyHandler() {
 * }
 *
 * MyHandler::~MyHandler() {
 * }
 *
 * esp_err_t MyHandler::apStart() {
 *   return ESP_OK;
 * }
 *
 * esp_err_t MyHandler::staConnected() {
 *   return ESP_OK;
 * }
 *
 * esp_err_t MyHandler::staDisconnected() {
 *   return ESP_OK;
 * }
 *
 * esp_err_t MyHandler::staStart() {
 *   return ESP_OK;
 * }
 *
 * esp_err_t MyHandler::staGotIp(system_event_sta_got_ip_t event_sta_got_ip) {
 *   return ESP_OK;
 * }
 *
 * esp_err_t MyHandler::wifiReady() {
 *   return ESP_OK;
 * }
 * @endcode
 */
class WiFiEventHandler {
public:
	WiFiEventHandler();
	virtual ~WiFiEventHandler();
	system_event_cb_t getEventHandler();
	virtual esp_err_t apStaConnected();
	virtual esp_err_t apStaDisconnected();
	virtual esp_err_t apStart();
	virtual esp_err_t apStop();
	virtual esp_err_t staConnected();
	virtual esp_err_t staDisconnected();
	virtual esp_err_t staGotIp(system_event_sta_got_ip_t event_sta_got_ip);
	virtual esp_err_t staStart();
	virtual esp_err_t staStop();
	virtual esp_err_t wifiReady();

	/**
	 * Get the next WiFi event handler in the chain, if there is one.
	 * @return The next WiFi event handler in the chain or nullptr if there is none.
	 */
	WiFiEventHandler *getNextHandler() {
		return nextHandler;
	}

	/**
	 * Set the next WiFi event handler in the chain.
	 * @param [in] nextHandler The next WiFi event handler in the chain.
	 */
	void setNextHandler(WiFiEventHandler* nextHandler) {
		this->nextHandler = nextHandler;
	}

private:
	WiFiEventHandler *nextHandler = nullptr;
	static esp_err_t eventHandler(void *ctx, system_event_t *event);
};

#endif /* MAIN_WIFIEVENTHANDLER_H_ */
