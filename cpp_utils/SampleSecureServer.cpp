/**
 * Create a new BLE server.
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

class MySecurity : public BLESecurityCallbacks {

	uint32_t onPassKeyRequest(){
		return 123456;
	}
	void onPassKeyNotify(uint32_t pass_key){
        ESP_LOGE(LOG_TAG, "The passkey Notify number:%d", pass_key);
	}
	bool onSecurityRequest(){
		return true;
	}
	void onAuthenticationComplete(esp_ble_auth_cmpl_t auth_cmpl){
		if(auth_cmpl.success){
			ESP_LOGI(LOG_TAG, "remote BD_ADDR:");
			esp_log_buffer_hex(LOG_TAG, auth_cmpl.bd_addr, sizeof(auth_cmpl.bd_addr));
			ESP_LOGI(LOG_TAG, "address type = %d", auth_cmpl.addr_type);
		}
        ESP_LOGI(LOG_TAG, "pair status = %s", auth_cmpl.success ? "success" : "fail");
	}
};

class MainBLEServer: public Task {
	void run(void *data) {
		ESP_LOGD(LOG_TAG, "Starting BLE work!");

		BLEDevice::init("ESP32");
		BLEServer* pServer = BLEDevice::createServer();
		pServer->setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
		pServer->setSecurityCallbacks(new MySecurity());

		BLEService* pService = pServer->createService("91bad492-b950-4226-aa2b-4ede9fa42f59");

		BLECharacteristic* pCharacteristic = pService->createCharacteristic(
			BLEUUID("0d563a58-196a-48ce-ace2-dfec78acc814"),
			BLECharacteristic::PROPERTY_BROADCAST | BLECharacteristic::PROPERTY_READ  |
			BLECharacteristic::PROPERTY_NOTIFY    | BLECharacteristic::PROPERTY_WRITE |
			BLECharacteristic::PROPERTY_INDICATE
		);
		pCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);

		pCharacteristic->setValue("Hello World!");

		BLE2902* p2902Descriptor = new BLE2902();
		p2902Descriptor->setNotifications(true);
		pCharacteristic->addDescriptor(p2902Descriptor);

		pService->start();

		BLEAdvertising* pAdvertising = pServer->getAdvertising();
		pAdvertising->addServiceUUID(BLEUUID(pService->getUUID()));

		BLESecurity *pSecurity = new BLESecurity();
		pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
		pSecurity->setCapability(ESP_IO_CAP_NONE);
		pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

		pAdvertising->start();

		ESP_LOGD(LOG_TAG, "Advertising started!");
		delay(1000000);
	}
};


void SampleSecureServer(void)
{
	//esp_log_level_set("*", ESP_LOG_DEBUG);
	MainBLEServer* pMainBleServer = new MainBLEServer();
	pMainBleServer->setStackSize(20000);
	pMainBleServer->start();

} // app_main
