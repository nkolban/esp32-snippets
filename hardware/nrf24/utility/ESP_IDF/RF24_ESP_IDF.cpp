#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sdkconfig.h"
#include "RF24_ESP_IDF.h"
#include "NRF24_spi.h"

void delay(int value) {
	vTaskDelay(value/portTICK_PERIOD_MS);
} // delay


void delayMicroseconds(int value) {
	return;
} // delayMicroseconds


void digitalWrite(int pin, int value) {
	gpio_set_level((gpio_num_t)pin, value);
} // digitalWrite


uint32_t millis() {
	return xTaskGetTickCount() * portTICK_PERIOD_MS;
} // millis


void pinMode(int pin, int mode) {
	gpio_config_t gpioConfig;
	gpioConfig.pin_bit_mask = (1 << pin);
	gpioConfig.mode         = mode==OUTPUT?GPIO_MODE_OUTPUT:GPIO_MODE_INPUT;
	gpioConfig.pull_up_en   = GPIO_PULLUP_DISABLE;
	gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
	gpioConfig.intr_type    = GPIO_INTR_DISABLE;
	gpio_config(&gpioConfig);
} // pinMode

NRF24_SPI _SPI;
