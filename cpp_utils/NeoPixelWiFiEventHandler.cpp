/*
 * NeoPixelWiFiEventHandler.cpp
 *
 *  Created on: Mar 1, 2017
 *      Author: kolban
 */
#include <stdio.h>
#include "NeoPixelWiFiEventHandler.h"

NeoPixelWiFiEventHandler::NeoPixelWiFiEventHandler(gpio_num_t gpioPin) {
	this->gpioPin = gpioPin;
	ws2812 = new WS2812(gpioPin, 8);
}

NeoPixelWiFiEventHandler::~NeoPixelWiFiEventHandler() {
	delete ws2812;
}

esp_err_t NeoPixelWiFiEventHandler::apStart() {
	printf("XXX apStart\n");
	ws2812->setPixel(0, 0, 00, 64);
	ws2812->show();
	return ESP_OK;
}

esp_err_t NeoPixelWiFiEventHandler::staConnected(system_event_sta_connected_t info) {
	printf("XXX staConnected\n");
	ws2812->setPixel(0, 57, 89, 66);
	ws2812->show();
	return ESP_OK;
}

esp_err_t NeoPixelWiFiEventHandler::staDisconnected(system_event_sta_disconnected_t info) {
	printf("XXX staDisconnected\n");
	ws2812->setPixel(0, 64, 0, 0);
	ws2812->show();
	return ESP_OK;
}

esp_err_t NeoPixelWiFiEventHandler::staStart() {
	printf("XXX staStart\n");
	ws2812->setPixel(0, 64, 64, 0);
	ws2812->show();
	return ESP_OK;
}

esp_err_t NeoPixelWiFiEventHandler::staGotIp(system_event_sta_got_ip_t info) {
	printf("XXX staGotIp\n");
	ws2812->setPixel(0, 0, 64, 0);
	ws2812->show();
	return ESP_OK;
}

esp_err_t NeoPixelWiFiEventHandler::wifiReady() {
	printf("XXX wifiReady\n");
	ws2812->setPixel(0, 64, 64, 0);
	ws2812->show();
	return ESP_OK;
}
