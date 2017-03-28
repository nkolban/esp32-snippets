/*
 * BLEService.cpp
 *
 *  Created on: Mar 25, 2017
 *      Author: kolban
 */
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <esp_log.h>

#include "BLEService.h"
#include "BLEUtils.h"

static char tag[] = "BLEService";

BLEService::BLEService() {
}

BLEService::~BLEService() {
}

/**
 * Dump details of this BLE GATT service.
 */
void BLEService::dump() {
	std::string name = "unknown";
	if (m_srvc_id.id.uuid.len == ESP_UUID_LEN_16) {
		name = BLEUtils::gattServiceToString(m_srvc_id.id.uuid.uuid.uuid16);
	}
	ESP_LOGD(tag, "%s [%s]", BLEUtils::gattServiceIdToString(m_srvc_id).c_str(), name.c_str());
} // dump
#endif // CONFIG_BT_ENABLED
