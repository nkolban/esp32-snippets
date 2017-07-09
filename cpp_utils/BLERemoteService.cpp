/*
 * BLERemoteService.cpp
 *
 *  Created on: Jul 8, 2017
 *      Author: kolban
 */
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include "BLERemoteService.h"

BLERemoteService::BLERemoteService(BLEUUID uuid) {
	m_uuid = uuid;
}

BLERemoteService::~BLERemoteService() {
	// TODO Auto-generated destructor stub
}

void BLERemoteService::getCharacteristics() {
}

#endif /* CONFIG_BT_ENABLED */
