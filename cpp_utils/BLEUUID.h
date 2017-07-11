/*
 * BLEUUID.h
 *
 *  Created on: Jun 21, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLEUUID_H_
#define COMPONENTS_CPP_UTILS_BLEUUID_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <esp_gatt_defs.h>
#include <string>

class BLEUUID {
public:
	BLEUUID(std::string uuid);
	BLEUUID(uint16_t uuid);
	BLEUUID(uint32_t uuid);
	BLEUUID(esp_bt_uuid_t uuid);
	BLEUUID(uint8_t *pData, size_t size, bool msbFirst);
	BLEUUID(esp_gatt_srvc_id_t srcvId);
	BLEUUID();
	virtual ~BLEUUID();
	bool           equals(BLEUUID uuid);
	esp_bt_uuid_t *getNative();
	void           to128();
	std::string    toString();

private:
	esp_bt_uuid_t m_uuid;
	bool          m_valueSet;
}; // BLEUUID
#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLEUUID_H_ */
