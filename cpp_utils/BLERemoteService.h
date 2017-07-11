/*
 * BLERemoteService.h
 *
 *  Created on: Jul 8, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLEREMOTESERVICE_H_
#define COMPONENTS_CPP_UTILS_BLEREMOTESERVICE_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include <map>

#include "BLEClient.h"
#include "BLERemoteCharacteristic.h"
#include "BLEUUID.h"
#include "FreeRTOS.h"

class BLEClient;
class BLERemoteCharacteristic;

class BLERemoteService {
public:
	BLERemoteService(esp_gatt_srvc_id_t srvcId, BLEClient* pClient);
	virtual ~BLERemoteService();

	// Public methods
	BLERemoteCharacteristic* getCharacteristic(BLEUUID uuid);
	void                     getCharacteristics(void);
	BLEClient*               getClient(void);
	BLEUUID                  getUUID(void);
	std::string              toString(void);

private:
	// Friends
	friend class BLEClient;
	friend class BLERemoteCharacteristic;

	// Private methods
	esp_gatt_srvc_id_t* getSrvcId(void);
	void                gattClientEventHandler(
		esp_gattc_cb_event_t      event,
		esp_gatt_if_t             gattc_if,
		esp_ble_gattc_cb_param_t* evtParam);
	void                removeCharacteristics();

	// Properties
	std::map<std::string, BLERemoteCharacteristic *> m_characteristicMap;
	bool                m_haveCharacteristics;
	BLEClient*          m_pClient;
	FreeRTOS::Semaphore m_semaphoreGetCharEvt = FreeRTOS::Semaphore("GetCharEvt");
	esp_gatt_srvc_id_t  m_srvcId;
	BLEUUID             m_uuid;
}; // BLERemoteService

#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLEREMOTESERVICE_H_ */
