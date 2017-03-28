/*
 * BLEService.h
 *
 *  Created on: Mar 25, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLESERVICE_H_
#define COMPONENTS_CPP_UTILS_BLESERVICE_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <esp_gattc_api.h>
class BLEService {
public:
	BLEService();
	virtual ~BLEService();
	void dump();
	void setService(esp_gatt_srvc_id_t srvc_id) {
		m_srvc_id = srvc_id;
	}
	esp_gatt_srvc_id_t getService() {
		return m_srvc_id;
	}

private:
	esp_gatt_srvc_id_t m_srvc_id;
};
#endif // CONFIG_BT_ENABLED
#endif /* COMPONENTS_CPP_UTILS_BLESERVICE_H_ */
