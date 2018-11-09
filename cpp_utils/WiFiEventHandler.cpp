/*
 * WiFiEventHandler.cpp
 *
 *  Created on: Feb 25, 2017
 *      Author: kolban
 */

#include "WiFiEventHandler.h"
#include <esp_event.h>
#include <esp_event_loop.h>
#include <esp_wifi.h>
#include <esp_err.h>
#include <esp_log.h>
#include "sdkconfig.h"

static const char* LOG_TAG = "WiFiEventHandler";

/**
 * @brief The entry point into the event handler.
 * Examine the event passed into the handler controller by the WiFi subsystem and invoke
 * the corresponding handler.
 * @param [in] ctx
 * @param [in] event
 * @return ESP_OK if the event was handled otherwise an error.
 */
esp_err_t WiFiEventHandler::eventHandler(void* ctx, system_event_t* event) {
	ESP_LOGD(LOG_TAG, ">> eventHandler called: ctx=0x%x, event=0x%x", (uint32_t) ctx, (uint32_t) event);
	WiFiEventHandler* pWiFiEventHandler = (WiFiEventHandler*) ctx;
	if (ctx == nullptr) {
		ESP_LOGD(LOG_TAG, "No context");
		return ESP_OK;
	}

	esp_err_t rc = ESP_OK;
	switch (event->event_id) {
		case SYSTEM_EVENT_AP_START: {
			rc =  pWiFiEventHandler->apStart();
			break;
		}

		case SYSTEM_EVENT_AP_STOP: {
			rc = pWiFiEventHandler->apStop();
			break;
		}

		case SYSTEM_EVENT_AP_STACONNECTED: {
			rc = pWiFiEventHandler->apStaConnected(event->event_info.sta_connected);
			break;
		}

		case SYSTEM_EVENT_AP_STADISCONNECTED: {
			rc = pWiFiEventHandler->apStaDisconnected(event->event_info.sta_disconnected);
			break;
		}

		case SYSTEM_EVENT_SCAN_DONE: {
			rc = pWiFiEventHandler->staScanDone(event->event_info.scan_done);
			break;
		}

		case SYSTEM_EVENT_STA_AUTHMODE_CHANGE: {
			rc = pWiFiEventHandler->staAuthChange(event->event_info.auth_change);
			break;
		}

		case SYSTEM_EVENT_STA_CONNECTED: {
			rc = pWiFiEventHandler->staConnected(event->event_info.connected);
			break;
		}

		case SYSTEM_EVENT_STA_DISCONNECTED: {
			rc = pWiFiEventHandler->staDisconnected(event->event_info.disconnected);
			break;
		}

		case SYSTEM_EVENT_STA_GOT_IP: {
			rc = pWiFiEventHandler->staGotIp(event->event_info.got_ip);
			break;
		}

		case SYSTEM_EVENT_STA_START: {
			rc = pWiFiEventHandler->staStart();
			break;
		}

		case SYSTEM_EVENT_STA_STOP: {
			rc = pWiFiEventHandler->staStop();
			break;
		}

		case SYSTEM_EVENT_WIFI_READY: {
			rc = pWiFiEventHandler->wifiReady();
			break;
		}

		default:
			break;
	}

	if (pWiFiEventHandler->m_nextHandler != nullptr) {
		ESP_LOGD(LOG_TAG, "Found a next handler");
		rc = eventHandler(pWiFiEventHandler->m_nextHandler, event);
	} else {
		//ESP_LOGD(LOG_TAG, "NOT Found a next handler");
	}
	return rc;
} // eventHandler


/**
 * @brief Constructor
 */
WiFiEventHandler::WiFiEventHandler() {
	m_nextHandler = nullptr;
} // WiFiEventHandler


/**
 * @brief Get the event handler.
 * Retrieve the event handler function to be passed to the ESP-IDF event handler system.
 * @return The event handler function.
 */
system_event_cb_t WiFiEventHandler::getEventHandler() {
	ESP_LOGD(LOG_TAG, ">> getEventHandler()");
	ESP_LOGD(LOG_TAG, "<< getEventHandler: 0x%x", (uint32_t)eventHandler);
  return eventHandler;
} // getEventHandler


/**
 * @brief Handle the Station Got IP event.
 * Handle having received/assigned an IP address when we are a station.
 * @param [in] event_sta_got_ip The Station Got IP event.
 * @return An indication of whether or not we processed the event successfully.
 */
esp_err_t WiFiEventHandler::staGotIp(system_event_sta_got_ip_t info) {
	ESP_LOGD(LOG_TAG, "default staGotIp");
	return ESP_OK;
} // staGotIp


/**
 * @brief Handle the Access Point started event.
 * Handle an indication that the ESP32 has started being an access point.
 * @return An indication of whether or not we processed the event successfully.
 */
esp_err_t WiFiEventHandler::apStart() {
	ESP_LOGD(LOG_TAG, "default apStart");
	return ESP_OK;
} // apStart


/**
 * @brief Handle the Access Point stop event.
 * Handle an indication that the ESP32 has stopped being an access point.
 * @return An indication of whether or not we processed the event successfully.
 */
esp_err_t WiFiEventHandler::apStop() {
	ESP_LOGD(LOG_TAG, "default apStop");
	return ESP_OK;
} // apStop


esp_err_t WiFiEventHandler::wifiReady() {
	ESP_LOGD(LOG_TAG, "default wifiReady");
	return ESP_OK;
} // wifiReady


esp_err_t WiFiEventHandler::staStart() {
	ESP_LOGD(LOG_TAG, "default staStart");
	return ESP_OK;
} // staStart


esp_err_t WiFiEventHandler::staStop() {
	ESP_LOGD(LOG_TAG, "default staStop");
	return ESP_OK;
} // staStop


/**
 * @brief Handle the Station Connected event.
 * Handle having connected to remote AP.
 * @param [in] event_connected system_event_sta_connected_t.
 * @return An indication of whether or not we processed the event successfully.
 */
esp_err_t WiFiEventHandler::staConnected(system_event_sta_connected_t info) {
	ESP_LOGD(LOG_TAG, "default staConnected");
	return ESP_OK;
} // staConnected


/**
 * @brief Handle the Station Disconnected event.
 * Handle having disconnected from remote AP.
 * @param [in] event_disconnected system_event_sta_disconnected_t.
 * @return An indication of whether or not we processed the event successfully.
 */
esp_err_t WiFiEventHandler::staDisconnected(system_event_sta_disconnected_t info) {
	ESP_LOGD(LOG_TAG, "default staDisconnected");
	return ESP_OK;
} // staDisconnected


/**
 * @brief Handle a Station Connected to AP event.
 * Handle having a station connected to ESP32 soft-AP.
 * @param [in] event_sta_connected system_event_ap_staconnected_t.
 * @return An indication of whether or not we processed the event successfully.
 */
esp_err_t WiFiEventHandler::apStaConnected(system_event_ap_staconnected_t info) {
	ESP_LOGD(LOG_TAG, "default apStaConnected");
	return ESP_OK;
} // apStaConnected


/**
 * @brief Handle a Station Disconnected from AP event.
 * Handle having a station disconnected from ESP32 soft-AP.
 * @param [in] event_sta_disconnected system_event_ap_stadisconnected_t.
 * @return An indication of whether or not we processed the event successfully.
 */
esp_err_t WiFiEventHandler::apStaDisconnected(system_event_ap_stadisconnected_t info) {
	ESP_LOGD(LOG_TAG, "default apStaDisconnected");
	return ESP_OK;
} // apStaDisconnected


/**
 * @brief Handle a Scan for APs done event.
 * Handle having an ESP32 station scan (APs) done.
 * @param [in] event_scan_done system_event_sta_scan_done_t.
 * @return An indication of whether or not we processed the event successfully.
 */
esp_err_t WiFiEventHandler::staScanDone(system_event_sta_scan_done_t info) {
	ESP_LOGD(LOG_TAG, "default staScanDone");
	return ESP_OK;
} // staScanDone


/**
 * @brief Handle the auth mode of APs change event.
 * Handle having the auth mode of AP ESP32 station connected to changed.
 * @param [in] event_auth_change system_event_sta_authmode_change_t.
 * @return An indication of whether or not we processed the event successfully.
 */
esp_err_t WiFiEventHandler::staAuthChange(system_event_sta_authmode_change_t info) {
	ESP_LOGD(LOG_TAG, "default staAuthChange");
	return ESP_OK;
} // staAuthChange


WiFiEventHandler::~WiFiEventHandler() {
} // ~WiFiEventHandler
