/*
 * BLEAdvertisedDeviceCallback.h
 *
 *  Created on: Jul 3, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLEADVERTISEDDEVICECALLBACKS_H_
#define COMPONENTS_CPP_UTILS_BLEADVERTISEDDEVICECALLBACKS_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include "BLEAdvertisedDevice.h"
class BLEAdvertisedDevice;

class BLEAdvertisedDeviceCallbacks {
public:
	BLEAdvertisedDeviceCallbacks();
	virtual ~BLEAdvertisedDeviceCallbacks();
	virtual void onResult(BLEAdvertisedDevice *pAdvertisedDevice) = 0;
};
#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLEADVERTISEDDEVICECALLBACKS_H_ */
