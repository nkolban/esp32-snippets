/*
 * BLEScan.h
 *
 *  Created on: Jul 1, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLESCAN_H_
#define COMPONENTS_CPP_UTILS_BLESCAN_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <esp_gap_ble_api.h>

#include "BLEAdvertisedDeviceCallbacks.h"
#include "BLEClient.h"

class BLEClient;
class BLEAdvertisedDeviceCallbacks;

class BLEScan {
public:
	BLEScan();
	virtual ~BLEScan();
	void gapEventHandler(
		esp_gap_ble_cb_event_t event,
		esp_ble_gap_cb_param_t *param);
	virtual void onResults();
	void         setActiveScan(bool active);
	void         setAdvertisedDeviceCallbacks(BLEAdvertisedDeviceCallbacks *pAdvertisedDeviceCallbacks);
	void         setInterval(uint16_t intervalMSecs);
	void         setWindow(uint16_t windowMSecs);
	void         start(uint32_t duration);
	void         stop();

private:
	friend class BLE;

	esp_ble_scan_params_t m_scan_params;
	void parseAdvertisement(BLEClient *pRemoteDevice, uint8_t *payload);
	BLEAdvertisedDeviceCallbacks *m_pAdvertisedDeviceCallbacks;
	bool m_stopped;
}; // BLEScan

#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLESCAN_H_ */
