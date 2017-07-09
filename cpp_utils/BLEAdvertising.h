/*
 * BLEAdvertising.h
 *
 *  Created on: Jun 21, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLEADVERTISING_H_
#define COMPONENTS_CPP_UTILS_BLEADVERTISING_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <esp_gap_ble_api.h>
#include "BLEUUID.h"
class BLEAdvertising {
public:
	BLEAdvertising();
	virtual ~BLEAdvertising();
	void start();
	void stop();
	void setAppearance(uint16_t appearance);
	void setServiceUUID(BLEUUID uuid);
private:
	esp_ble_adv_data_t   m_advData;
	esp_ble_adv_params_t m_advParams;
	BLEUUID              m_serviceUUID;
};
#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLEADVERTISING_H_ */
