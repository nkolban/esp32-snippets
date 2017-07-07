/*
 * BLEClientCallbacks.cpp
 *
 *  Created on: Jul 5, 2017
 *      Author: kolban
 */

#include "BLEClientCallbacks.h"
#include <esp_log.h>
static const char LOG_TAG[] = "BLEClientCallbacks";

BLEClientCallbacks::BLEClientCallbacks() {
	// TODO Auto-generated constructor stub

}

BLEClientCallbacks::~BLEClientCallbacks() {
	// TODO Auto-generated destructor stub
}

void BLEClientCallbacks::onConnect(BLEClient* pClient) {
	ESP_LOGD(LOG_TAG, ">> onConnect(): Default");
	ESP_LOGD(LOG_TAG, "<< onConnect()");
}
