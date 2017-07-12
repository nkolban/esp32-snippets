#include "BLE.h"
#include "BLEUtils.h"
#include "BLEScan.h"
#include <esp_log.h>
#include <string>

#include "BLEAdvertisedDeviceCallbacks.h"
#include "BLEClient.h"
#include "sdkconfig.h"
#include "Task.h"

static const char LOG_TAG[] = "SampleClient";

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/



static BLEUUID serviceUUID("91bad492-b950-4226-aa2b-4ede9fa42f59");
static BLEUUID    charUUID("0d563a58-196a-48ce-ace2-dfec78acc814");

class MyClient: public Task {
	void run(void *data) {
		BLEAddress* pAddress = (BLEAddress *)data;
		BLEClient*  pClient = BLE::createClient();


		pClient->connect(*pAddress);
		pClient->getServices();

		BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
		if (pRemoteService == nullptr) {
			ESP_LOGD(LOG_TAG, "Failed to find our service UUID: %s", serviceUUID.toString().c_str());
			return;
		}

		pRemoteService->getCharacteristics();
		BLERemoteCharacteristic* pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
		if (pRemoteCharacteristic == nullptr) {
			ESP_LOGD(LOG_TAG, "Failed to find our characteristic UUID: %s", charUUID.toString().c_str());
			return;
		}
		pRemoteCharacteristic->readValue();

		pRemoteCharacteristic->writeValue("123");
		pRemoteCharacteristic->registerForNotify();
		pClient->disconnect();

		ESP_LOGD(LOG_TAG, "%s", pClient->toString().c_str());
		ESP_LOGD(LOG_TAG, "-- End of task");
	}
};



class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
	void onResult(BLEAdvertisedDevice *pAdvertisedDevice) {
		ESP_LOGD(LOG_TAG, "Advertised Device: %s", pAdvertisedDevice->toString().c_str());
		if (pAdvertisedDevice->haveServiceUUID()) {
			ESP_LOGD(LOG_TAG, "Comparing %s to %s",
				pAdvertisedDevice->getServiceUUID().toString().c_str(), serviceUUID.toString().c_str());
		}
		if (pAdvertisedDevice->haveServiceUUID() && pAdvertisedDevice->getServiceUUID().equals(serviceUUID)) {
			pAdvertisedDevice->getScan()->stop();
			ESP_LOGD(LOG_TAG, "Found our device!  address: %s", pAdvertisedDevice->getAddress().toString().c_str());
			MyClient *pMyClient = new MyClient();
			pMyClient->setStackSize(18000);
			pMyClient->start(new BLEAddress(*pAdvertisedDevice->getAddress().getNative()));

		}
	}
};



static void run() {
	ESP_LOGD(LOG_TAG, "Scanning sample starting");
	BLEUUID x = BLEUUID("12345678-90ab-cdef-1234-567890abcdef");
	ESP_LOGD(LOG_TAG, "%s", x.toString().c_str());
	BLE::initClient();
	BLEScan *pBLEScan = BLE::getScan();
	pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
	pBLEScan->setActiveScan(true);
	pBLEScan->start(30);

	//BLEClient *pClient = BLE::createClient();
	//pClient->setClientCallbacks(new MyClientCallbacks());
	//pClient->connect(BLEAddress("00:00:00:00:00:00"));

}

void SampleClient(void)
{
	run();
} // app_main
