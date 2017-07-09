/*
 * BLEServerCallbacks.cpp
 *
 *  Created on: Jul 4, 2017
 *      Author: kolban
 */
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include "BLEServerCallbacks.h"
#include <esp_log.h>
static const char LOG_TAG[] = "BLEServerCallbacks";

BLEServerCallbacks::BLEServerCallbacks() {
	// TODO Auto-generated constructor stub

}

BLEServerCallbacks::~BLEServerCallbacks() {
	// TODO Auto-generated destructor stub
}

void BLEServerCallbacks::onConnect(BLEServer* pServer) {
	ESP_LOGD(LOG_TAG, ">> onConnect(): Default");
	ESP_LOGD(LOG_TAG, "<< onConnect()");
}

void BLEServerCallbacks::onDisconnect(BLEServer* pServer) {
	ESP_LOGD(LOG_TAG, ">> onDisconnect(): Default");
	ESP_LOGD(LOG_TAG, "<< onDisconnect()");
}
#endif /* CONFIG_BT_ENABLED */
