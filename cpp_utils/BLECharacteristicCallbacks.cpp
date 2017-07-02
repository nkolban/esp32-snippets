/*
 * BLECharacteristicCallbacks.cpp
 *
 *  Created on: Jul 2, 2017
 *      Author: kolban
 */

#include "BLECharacteristicCallbacks.h"
#include <esp_log.h>
static char LOG_TAG[] = "BLECharacteristicCallbacks";

BLECharacteristicCallbacks::BLECharacteristicCallbacks() {
	// TODO Auto-generated constructor stub

}

BLECharacteristicCallbacks::~BLECharacteristicCallbacks() {
	// TODO Auto-generated destructor stub
}

/**
 * @brief Callback function to support a read request.
 */
void BLECharacteristicCallbacks::onRead(BLECharacteristic *pCharacteristic) {
	ESP_LOGD(LOG_TAG, ">> onRead: default");
	ESP_LOGD(LOG_TAG, "<< onRead");
} // onRead


/**
 * @brief Callback function to support a write request.
 */
void BLECharacteristicCallbacks::onWrite(BLECharacteristic *pCharacteristic) {
	ESP_LOGD(LOG_TAG, ">> onWrite: default");
	ESP_LOGD(LOG_TAG, "<< onWrite");
} // onWrite
