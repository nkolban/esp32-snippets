/*
 * SampleServer_Encryption.cpp
 *
 *  Created on: Dec 23, 2017
 *      Author: chegewara
 */

#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLE2902.h"
#include <esp_log.h>
#include <string>
#include <Task.h>


#include "sdkconfig.h"

static char LOG_TAG[] = "SampleServer";

class MyCharacteristicCallback: public BLECharacteristicCallbacks {
	void onWrite(BLECharacteristic* pCharacteristic) {
		std::string msg = pCharacteristic->getValue();
		ESP_LOGI(LOG_TAG, "BLE received: %s", msg.c_str());
	}

	void onRead(BLECharacteristic* pCharacteristic) {
		std::string msg = pCharacteristic->getValue();
		ESP_LOGI(LOG_TAG, "BLE received: %s, %i", msg.c_str(), msg.length());
	}
};

class MainBLEServer: public Task {
	void run(void *data) {
		ESP_LOGD(LOG_TAG, "Starting BLE work!");
		esp_log_buffer_char(LOG_TAG, LOG_TAG, sizeof(LOG_TAG));
		esp_log_buffer_hex(LOG_TAG, LOG_TAG, sizeof(LOG_TAG));
		BLEDevice::init("ESP32");
		BLEServer* pServer = BLEDevice::createServer();
		/*
		 * Here we have implemented simplest security. This kind security does not provide authentication
		 */
		BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);

		BLEService* pService = pServer->createService("91bad492-b950-4226-aa2b-4ede9fa42f59");

		BLECharacteristic* pCharacteristic = pService->createCharacteristic(
			BLEUUID("0d563a58-196a-48ce-ace2-dfec78acc814"),
			BLECharacteristic::PROPERTY_READ  		|
			BLECharacteristic::PROPERTY_NOTIFY    	|
			BLECharacteristic::PROPERTY_WRITE 		|
			BLECharacteristic::PROPERTY_INDICATE
		);

		pCharacteristic->setCallbacks(new MyCharacteristicCallback());
		pCharacteristic->setValue("Hello World!");

		BLE2902* p2902Descriptor = new BLE2902();
		p2902Descriptor->setNotifications(true);
		pCharacteristic->addDescriptor(p2902Descriptor);

		pService->start();

		BLEAdvertising* pAdvertising = pServer->getAdvertising();
		pAdvertising->addServiceUUID(BLEUUID(pService->getUUID()));

		pAdvertising->start();

		ESP_LOGD(LOG_TAG, "Advertising started!");
		delay(portMAX_DELAY);
	}
};


void SampleServer_Encryption(void)
{
	//esp_log_level_set("*", ESP_LOG_DEBUG);
	MainBLEServer* pMainBleServer = new MainBLEServer();
	pMainBleServer->setStackSize(20000);
	pMainBleServer->start();

} // app_main



