/*
 * GPIO.cpp
 *
 *  Created on: Feb 28, 2017
 *      Author: kolban
 */

#include "GPIO.h"
#include <driver/gpio.h>
#include "sdkconfig.h"
#include <esp_log.h>
#include <esp_err.h>
#include "GeneralUtils.h"

static const char* LOG_TAG = "GPIO";

static bool g_isrServiceInstalled = false;

/**
 * @brief Add an ISR handler to the pin.
 * @param [in] pin The pin to have the ISR associated with it.
 * @param [in] handler The function to be invoked when the interrupt is detected.
 * @param [in] pArgs Optional arguments to pass to the handler.
 */
void ESP32CPP::GPIO::addISRHandler(gpio_num_t pin, gpio_isr_t handler, void* pArgs) {
	ESP_LOGD(LOG_TAG, ">> addISRHandler:  pin=%d", pin);

	// If we have not yet installed the ISR service handler, install it now.
	if (!g_isrServiceInstalled) {
		ESP_LOGD(LOG_TAG, "Installing the global ISR service");
		esp_err_t errRc = ::gpio_install_isr_service(0);
		if (errRc != ESP_OK) {
			ESP_LOGE(LOG_TAG, "<< gpio_install_isr_service: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
			return;
		}
		g_isrServiceInstalled = true;
	}

	esp_err_t errRc = ::gpio_isr_handler_add(pin, handler, pArgs);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "<< gpio_isr_handler_add: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}

	ESP_LOGD(LOG_TAG, "<< addISRHandler");
} // addISRHandler


/**
 * @brief Set the pin high.
 *
 * Ensure that the pin is set to be output prior to calling this method.
 *
 * @param [in] pin The pin to be set high.
 * @return N/A.
 */
void ESP32CPP::GPIO::high(gpio_num_t pin) {
	write(pin, true);
} // high


/**
 * @brief Determine if the pin is a valid pin for an ESP32 (i.e. is it in range).
 *
 * @param [in] pin The pin number to validate.
 * @return The value of true if the pin is valid and false otherwise.
 */
bool ESP32CPP::GPIO::inRange(gpio_num_t pin) {
	return (pin >= 0 && pin <= 39);
} // inRange


/**
 * @brief Disable interrupts on the named pin.
 * @param [in] pin The pin to disable interrupts upon.
 * @return N/A.
 */
void ESP32CPP::GPIO::interruptDisable(gpio_num_t pin) {
	esp_err_t rc = ::gpio_intr_disable(pin);
	if (rc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "interruptDisable: %d", rc);
	}
} // interruptDisable


/**
 * @brief Enable interrupts on the named pin.
 * @param [in] pin The pin to enable interrupts upon.
 * @return N/A.
 */
void ESP32CPP::GPIO::interruptEnable(gpio_num_t pin) {
	esp_err_t rc = ::gpio_intr_enable(pin);
	if (rc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "interruptEnable: %d", rc);
	}
} // interruptEnable


/**
 * @brief Set the pin low.
 *
 * Ensure that the pin is set to be output prior to calling this method.
 *
 * @param [in] pin The pin to be set low.
 * @return N/A.
 */
void ESP32CPP::GPIO::low(gpio_num_t pin) {
	write(pin, false);
} // low


/**
 * @brief Read a value from the given pin.
 *
 * Ensure the pin is set as input before calling this method.
 * @param [in] pin The pin to read from.
 * @return True if the pin is high, false if the pin is low.
 */
bool ESP32CPP::GPIO::read(gpio_num_t pin) {
	return ::gpio_get_level(pin) == 1;
} // read


/**
 * @brief Set the pin as input.
 *
 * Set the direction of the pin as input.
 * @param [in] pin The pin to set as input.
 * @return N/A.
 */
void ESP32CPP::GPIO::setInput(gpio_num_t pin) {
	::gpio_set_direction(pin, GPIO_MODE_INPUT);
} // setInput


/**
 * @brief Set the interrupt type.
 * The type of interrupt can be one of:
 *
 * * GPIO_INTR_ANYEDGE
 * * GPIO_INTR_DISABLE
 * * GPIO_INTR_NEGEDGE
 * * GPIO_INTR_POSEDGE
 * * GPIO_INTR_LOW_LEVEL
 * * GPIO_INTR_HIGH_LEVEL
 *
 * @param [in] pin The pin to set the interrupt upon.
 * @param [in] intrType The type of interrupt.
 * @return N/A.
 */
void ESP32CPP::GPIO::setInterruptType(gpio_num_t pin, gpio_int_type_t intrType) {
	esp_err_t rc = ::gpio_set_intr_type(pin, intrType);
	if (rc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "setInterruptType: %d", rc);
	}
} // setInterruptType


/**
 * @brief Set the pin as output.
 *
 * Set the direction of the pin as output.  Note that pins 34 through 39 are input only and can **not** be
 * set as output.
 * @param [in] pin The pin to set as output.
 * @return N/A.
 */
void ESP32CPP::GPIO::setOutput(gpio_num_t pin) {
	::gpio_set_direction(pin, GPIO_MODE_OUTPUT);
} // setOutput


/**
 * @brief Write a value to the given pin.
 *
 * Ensure that the pin is set as output before calling this method.
 * @param [in] pin The gpio pin to change.
 * @param [out] value The value to be written to the pin.
 * @return N/A.
 */
void ESP32CPP::GPIO::write(gpio_num_t pin, bool value) {
	//ESP_LOGD(LOG_TAG, ">> write: pin: %d, value: %d", pin, value);
	esp_err_t errRc = ::gpio_set_level(pin, value ? 1 : 0);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "<< gpio_set_level: pin=%d, rc=%d %s", pin, errRc, GeneralUtils::errorToString(errRc));
	}
} // write


/**
 * @brief Write up to 8 bits of data to a set of pins.
 * @param [in] pins An array of pins to set their values.
 * @param [in] value The data value to write.
 * @param [in] bits The number of bits to write.
 */
void ESP32CPP::GPIO::writeByte(gpio_num_t pins[], uint8_t value, int bits) {
	ESP_LOGD(LOG_TAG, ">> writeByte: value: %.2x, bits: %d", value, bits);
	for (int i = 0; i < bits; i++) {
		//ESP_LOGD(LOG_TAG, "i=%d, bits=%d", i, bits);
		write(pins[i], (value & (1 << i)) != 0);
	}
	ESP_LOGD(LOG_TAG, "<< writeByte");
} // writeByte
