/*
 * NeoPixelWiFiEventHandler.cpp
 *
 *  Created on: Mar 1, 2017
 *      Author: kolban
 */
#include <stdio.h>
#include <esp_log.h>
#include "NeoPixelWiFiEventHandler.h"

static const char* LOG_TAG = "NeoPixelWiFiEventHandler";

NeoPixelWiFiEventHandler::NeoPixelWiFiEventHandler(gpio_num_t gpioPin) {
	this->gpioPin = gpioPin;
	ws2812 = new WS2812(gpioPin, 8);
}

NeoPixelWiFiEventHandler::~NeoPixelWiFiEventHandler() {
	delete ws2812;
}

esp_err_t NeoPixelWiFiEventHandler::apStart() {
	ESP_LOGD(LOG_TAG, "XXX apStart");
	ws2812->setPixel(0, 0, 00, 64);
	ws2812->show();
	return ESP_OK;
}

esp_err_t NeoPixelWiFiEventHandler::staConnected(system_event_sta_connected_t info) {
	ESP_LOGD(LOG_TAG, "XXX staConnected");
	ws2812->setPixel(0, 57, 89, 66);
	ws2812->show();
	return ESP_OK;
}

esp_err_t NeoPixelWiFiEventHandler::staDisconnected(system_event_sta_disconnected_t info) {
	ESP_LOGD(LOG_TAG, "XXX staDisconnected");
	ws2812->setPixel(0, 64, 0, 0);
	ws2812->show();
	return ESP_OK;
}

esp_err_t NeoPixelWiFiEventHandler::staStart() {
	ESP_LOGD(LOG_TAG, "XXX staStart");
	ws2812->setPixel(0, 64, 64, 0);
	ws2812->show();
	return ESP_OK;
}

esp_err_t NeoPixelWiFiEventHandler::staGotIp(system_event_sta_got_ip_t info) {
	ESP_LOGD(LOG_TAG, "XXX staGotIp");
	ws2812->setPixel(0, 0, 64, 0);
	ws2812->show();
	return ESP_OK;
}

esp_err_t NeoPixelWiFiEventHandler::wifiReady() {
	ESP_LOGD(LOG_TAG, "XXX wifiReady");
	ws2812->setPixel(0, 64, 64, 0);
	ws2812->show();
	return ESP_OK;
}
