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

static char tag[] = "WiFiEventHandler";

static esp_err_t eventHandler(void *ctx, system_event_t *event) {
	ESP_LOGD(tag, "eventHandler called");
	WiFiEventHandler *pWiFiEventHandler = (WiFiEventHandler *)ctx;
	if (ctx == nullptr) {
		ESP_LOGD(tag, "No context");
		return ESP_OK;
	}
	switch(event->event_id) {
		case SYSTEM_EVENT_STA_GOT_IP:
			return pWiFiEventHandler->staGotIp(event->event_info.got_ip);
		case SYSTEM_EVENT_AP_START:
			return pWiFiEventHandler->apStart();
		case SYSTEM_EVENT_AP_STOP:
			return pWiFiEventHandler->apStop();
		case SYSTEM_EVENT_STA_CONNECTED:
			return pWiFiEventHandler->staConnected();
		case SYSTEM_EVENT_STA_DISCONNECTED:
			return pWiFiEventHandler->staDisconnected();
		case SYSTEM_EVENT_STA_START:
			return pWiFiEventHandler->staStart();
		case SYSTEM_EVENT_STA_STOP:
			return pWiFiEventHandler->staStop();
		default:
			break;
	}
	return ESP_OK;
}

WiFiEventHandler::WiFiEventHandler() {
}


/**
 * Retrieve the event handler function to be passed to the ESP-IDF event handler system.
 * @return The event handler function.
 */
system_event_cb_t WiFiEventHandler::getEventHandler() {
	return eventHandler;
}


/**
 * Handle having received/assigned an IP address when we are a station.
 * @return An indication of whether or not we processed the event successfully.
 */
esp_err_t WiFiEventHandler::staGotIp(
		system_event_sta_got_ip_t event_sta_got_ip) {
	ESP_LOGD(tag, "default staGotIp");
	return ESP_OK;
}

esp_err_t WiFiEventHandler::apStart() {
	ESP_LOGD(tag, "default apStart");
	return ESP_OK;
}

esp_err_t WiFiEventHandler::apStop() {
	ESP_LOGD(tag, "default apStop");
	return ESP_OK;
}

esp_err_t WiFiEventHandler::wifiReady() {
	ESP_LOGD(tag, "default wifiReady");
	return ESP_OK;
}

esp_err_t WiFiEventHandler::staStart() {
	ESP_LOGD(tag, "default staStart");
	return ESP_OK;
}

esp_err_t WiFiEventHandler::staStop() {
	ESP_LOGD(tag, "default staStop");
	return ESP_OK;
}

esp_err_t WiFiEventHandler::staConnected() {
	ESP_LOGD(tag, "default staConnected");
	return ESP_OK;
}

esp_err_t WiFiEventHandler::staDisconnected() {
	ESP_LOGD(tag, "default staDisconnected");
	return ESP_OK;
}

esp_err_t WiFiEventHandler::apStaConnected() {
	ESP_LOGD(tag, "default apStaConnected");
	return ESP_OK;
}

esp_err_t WiFiEventHandler::apStaDisconnected() {
	ESP_LOGD(tag, "default apStaDisconnected");
	return ESP_OK;
}
