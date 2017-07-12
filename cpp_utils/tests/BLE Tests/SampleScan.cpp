#include "BLE.h"
#include "BLEUtils.h"
#include "BLEScan.h"
#include <esp_log.h>
#include <string>

#include "BLEAdvertisedDeviceCallbacks.h"
#include "sdkconfig.h"

static const char LOG_TAG[] = "SampleScan";

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
	void onResult(BLEAdvertisedDevice *pAdvertisedDevice) {
		ESP_LOGD(LOG_TAG, "Advertised Device: %s", pAdvertisedDevice->toString().c_str());
	}
};

static void run() {
	ESP_LOGD(LOG_TAG, "Scanning sample starting");
	BLE::initClient();
	BLEScan* pBLEScan = BLE::getScan();
	pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
	pBLEScan->setActiveScan(true);
	std::vector<BLEAdvertisedDevice*> foundDevices = pBLEScan->start(30);
	ESP_LOGD(LOG_TAG, "We found %d devices", foundDevices.size());
	ESP_LOGD(LOG_TAG, "Scanning sample ended");
}

void SampleScan(void)
{
	run();
} // app_main
