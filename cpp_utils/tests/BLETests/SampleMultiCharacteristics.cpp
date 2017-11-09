/**
 * Create a new BLE server.
 */
//#include "freertos/FreeRTOS.h"
#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLE2902.h"
#include <esp_log.h>
#include <string>
#include <Task.h>
#include "sdkconfig.h"

#define SERVICE_UUID_128 "91bad492-b950-4226-aa2b-4ede9fa42f59"
#define CHARACTERISTIC_UUID_128 "0d563a58-196a-48ce-ace2-dfec78acc814"

static char LOG_TAG[] = "SampleServer";

class MainBLEServer: public Task {
	void run(void *data) {
		ESP_LOGD(LOG_TAG, "Starting BLE work!");

		BLEDevice::init("");
		BLEServer* pServer = BLEDevice::createServer();

		BLEService* pService = pServer->createService(SERVICE_UUID_128);

		BLECharacteristic* pCharacteristic = pService->createCharacteristic(
			BLEUUID(CHARACTERISTIC_UUID_128),
			BLECharacteristic::PROPERTY_BROADCAST | BLECharacteristic::PROPERTY_READ  |
			BLECharacteristic::PROPERTY_NOTIFY    | BLECharacteristic::PROPERTY_WRITE |
			BLECharacteristic::PROPERTY_INDICATE
		);
		pCharacteristic->setValue("Hello World! Characteristic no 1!");

		pCharacteristic = pService->createCharacteristic(
			BLEUUID(CHARACTERISTIC_UUID_128),
			BLECharacteristic::PROPERTY_BROADCAST | BLECharacteristic::PROPERTY_READ  |
			BLECharacteristic::PROPERTY_NOTIFY    | BLECharacteristic::PROPERTY_WRITE |
			BLECharacteristic::PROPERTY_INDICATE
		);

		pCharacteristic->setValue("Hello World! Characteristic no 2!");

		pCharacteristic = pService->createCharacteristic(
			BLEUUID(CHARACTERISTIC_UUID_128),
			BLECharacteristic::PROPERTY_BROADCAST | BLECharacteristic::PROPERTY_READ  |
			BLECharacteristic::PROPERTY_NOTIFY    | BLECharacteristic::PROPERTY_WRITE |
			BLECharacteristic::PROPERTY_INDICATE
		);

		pCharacteristic->setValue("Hello World! Characteristic no 3!");

		BLE2902* p2902Descriptor = new BLE2902();
		p2902Descriptor->setNotifications(true);
		pCharacteristic->addDescriptor(p2902Descriptor);
		pService->start();

		BLEAdvertising* pAdvertising = pServer->getAdvertising();
		/*
		 * To add 128bit service UUID to advertising  REMEMBER:
		 * BLEDevice::init("ESP32"); in this line
		 * name should be very short, in most cases no longer than 5 characters.
		 * In this case we have also setup appearance, so we need to leave device name empty
		 */
		pAdvertising->addServiceUUID(BLEUUID(pService->getUUID()));
		/*
		 * Setting appearance
		 * https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.gap.appearance.xml
		 */
		pAdvertising->setAppearance(0x1280);
		pAdvertising->start();

		ESP_LOGD(LOG_TAG, "Advertising started!");
		delay(1000000);
	}
};


void SampleMultiCharacteristic(void)
{
	//esp_log_level_set("*", ESP_LOG_DEBUG);
	MainBLEServer* pMainBleServer = new MainBLEServer();
	pMainBleServer->setStackSize(4000);
	pMainBleServer->start();

} // app_main
