/*
 * BLECharacteristicCallbacks.cpp
 *
 *  Created on: Jul 2, 2017
 *      Author: kolban
 */
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include "BLECharacteristic.h"
#include <esp_log.h>
static const char* LOG_TAG = "BLECharacteristicCallbacks";


BLECharacteristicCallbacks::~BLECharacteristicCallbacks() {}

/**
 * @brief Callback function to support a read request.
 * @param [in] pCharacteristic The characteristic that is the source of the event.
 */
void BLECharacteristicCallbacks::onRead(BLECharacteristic *pCharacteristic) {
	ESP_LOGD(LOG_TAG, ">> onRead: default");
	ESP_LOGD(LOG_TAG, "<< onRead");
} // onRead


/**
 * @brief Callback function to support a write request.
 * @param [in] pCharacteristic The characteristic that is the source of the event.
 */
void BLECharacteristicCallbacks::onWrite(BLECharacteristic *pCharacteristic) {
	ESP_LOGD(LOG_TAG, ">> onWrite: default");
	ESP_LOGD(LOG_TAG, "<< onWrite");
} // onWrite
#endif /* CONFIG_BT_ENABLED */
