/*
 * BLEAdvertisedDeviceCallback.cpp
 *
 *  Created on: Jul 3, 2017
 *      Author: kolban
 */
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include "BLEAdvertisedDeviceCallbacks.h"

BLEAdvertisedDeviceCallbacks::BLEAdvertisedDeviceCallbacks() {
}

BLEAdvertisedDeviceCallbacks::~BLEAdvertisedDeviceCallbacks() {
}
#endif /* CONFIG_BT_ENABLED */
