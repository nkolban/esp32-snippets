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
 * * esp_err_t apStaConnected(system_event_ap_staconnected_t info)
 * * esp_err_t apStaDisconnected(system_event_ap_stadisconnected_t info)
 * * esp_err_t apStart()
 * * esp_err_t apStop()
 * * esp_err_t staConnected(system_event_sta_connected_t info)
 * * esp_err_t staDisconnected(system_event_sta_disconnected_t info)
 * * esp_err_t staGotIp(system_event_sta_got_ip_t info)
 * * esp_err_t staScanDone(system_event_sta_scan_done_t info)
 * * esp_err_t staAuthChange(system_event_sta_authmode_change_t info)
 * * esp_err_t staStart()
 * * esp_err_t staStop()
 * * esp_err_t wifiReady()
 */

#ifndef MAIN_WIFIEVENTHANDLER_H_
#define MAIN_WIFIEVENTHANDLER_H_
#include <esp_event.h>
#include <esp_event_loop.h>

/**
 * @brief %WiFi state event handler.
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
 * esp_err_t MyHandler::staConnected(system_event_sta_connected_t info) {
 *   return ESP_OK;
 * }
 *
 * esp_err_t MyHandler::staDisconnected(system_event_sta_disconnected_t info) {
 *   return ESP_OK;
 * }
 *
 * esp_err_t MyHandler::apStaConnected(system_event_ap_staconnected_t info) {
 *   return ESP_OK;
 * }
 *
 * esp_err_t MyHandler::apStaDisconnected(system_event_ap_stadisconnected_t info) {
 *   return ESP_OK;
 * }
 *
 * esp_err_t MyHandler::staStart() {
 *   return ESP_OK;
 * }
 *
 * esp_err_t MyHandler::staGotIp(system_event_sta_got_ip_t info) {
 *   return ESP_OK;
 * }
 *
 * esp_err_t MyHandler::staScanDone(system_event_sta_scan_done_t info) {
 *   return ESP_OK;
 * }
 *
 * esp_err_t MyHandler::staAuthChange(system_event_sta_authmode_change_t info) {
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
	virtual esp_err_t apStaConnected(system_event_ap_staconnected_t info);
	virtual esp_err_t apStaDisconnected(system_event_ap_stadisconnected_t info);
	virtual esp_err_t apStart();
	virtual esp_err_t apStop();
	system_event_cb_t getEventHandler();
	virtual esp_err_t staConnected(system_event_sta_connected_t info);
	virtual esp_err_t staDisconnected(system_event_sta_disconnected_t info);
	virtual esp_err_t staGotIp(system_event_sta_got_ip_t info);
	virtual esp_err_t staScanDone(system_event_sta_scan_done_t info);
	virtual esp_err_t staAuthChange(system_event_sta_authmode_change_t info);
	virtual esp_err_t staStart();
	virtual esp_err_t staStop();
	virtual esp_err_t wifiReady();

	/**
	 * Get the next WiFi event handler in the chain, if there is one.
	 * @return The next WiFi event handler in the chain or nullptr if there is none.
	 */
	WiFiEventHandler* getNextHandler() {
		return m_nextHandler;
	}

	/**
	 * Set the next WiFi event handler in the chain.
	 * @param [in] nextHandler The next WiFi event handler in the chain.
	 */
	void setNextHandler(WiFiEventHandler* nextHandler) {
		this->m_nextHandler = nextHandler;
	}

private:
	friend class WiFi;
	WiFiEventHandler *m_nextHandler;
	static esp_err_t eventHandler(void* ctx, system_event_t* event);

};

#endif /* MAIN_WIFIEVENTHANDLER_H_ */
