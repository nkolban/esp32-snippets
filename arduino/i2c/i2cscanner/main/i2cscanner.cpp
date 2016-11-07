/**
 * Scan for I2C devices.
 *
 * Scan for I2C devices using the Arduino Wire library.
 *
 * For additional details and documentation see:
 * * Free book on ESP32 - https://leanpub.com/kolban-ESP32
 *
 *
 * Neil Kolban <kolban1@kolban.com>
 *
 */
#include "Wire.h"
#include "esp_log.h"
#include "driver/gpio.h"

#define SDAPIN (GPIO_NUM_22)
#define SCLPIN (GPIO_NUM_23)

static char tag[]="i2cscanner";

extern "C" {
	void I2CScanner();
}

void I2CScanner() {
	ESP_LOGI(tag, "I2C scanning with SDA=%d, CLK=%d", SDAPIN, SCLPIN);
	Wire.begin(SDAPIN, SCLPIN);
	int address;
	int foundCount = 0;
	for (address=1; address<127; address++) {
		Wire.beginTransmission(address);
		uint8_t error = Wire.endTransmission();
		if (error == 0) {
			foundCount++;
			ESP_LOGI(tag, "Found device at 0x%.2x", address);
		}
	}
	ESP_LOGI(tag, "Found %d I2C devices by scanning.", foundCount);
}
