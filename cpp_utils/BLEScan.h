/*
 * BLEScan.h
 *
 *  Created on: Jul 1, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLESCAN_H_
#define COMPONENTS_CPP_UTILS_BLESCAN_H_
#include <esp_gap_ble_api.h>

class BLEScan {
public:
	BLEScan();
	virtual ~BLEScan();
	virtual void onResults();
	void setActiveScan(bool active);
	void setInterval(uint16_t intervalMSecs);
	void setWindow(uint16_t windowMSecs);
	void start(uint32_t duration);
	void stop();

private:
	esp_ble_scan_params_t m_scan_params;
}; // BLEScan

#endif /* COMPONENTS_CPP_UTILS_BLESCAN_H_ */
