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

static char tag[] = "GPIO";

/**
 * @brief Class instance constructor.
 */
/*
GPIO::GPIO() {
}
*/

/**
 * @brief Determine if the pin is a valid pin for an ESP32 (i.e. is it in range).
 *
 * @param [in] pin The pin number to validate.
 * @return The value of true if the pin is valid and false otherwise.
 */
bool ESP32CPP::GPIO::inRange(gpio_num_t pin) {
	if (pin>=0 && pin<=39) {
		return true;
	}
	return false;
} // inRange


/**
 * @brief Disable interrupts on the named pin.
 * @param [in] pin The pin to disable interrupts upon.
 * @return N/A.
 */
void ESP32CPP::GPIO::interruptDisable(gpio_num_t pin) {
	esp_err_t rc = ::gpio_intr_disable(pin);
	if (rc != ESP_OK) {
		ESP_LOGE(tag, "interruptDisable: %d", rc);
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
		ESP_LOGE(tag, "interruptEnable: %d", rc);
	}
} // interruptEnable


/**
 * @brief Read a value from the given pin.
 *
 * Ensure the pin is set as input before calling this method.
 * @param [in] pin The pin to read from.
 * @return True if the pin is high, false if the pin is low.
 */
bool ESP32CPP::GPIO::read(gpio_num_t pin) {
	return ::gpio_get_level(pin);
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
void ESP32CPP::GPIO::setInterruptType(
		gpio_num_t pin,
		gpio_int_type_t intrType) {
	esp_err_t rc = ::gpio_set_intr_type(pin, intrType);
	if (rc != ESP_OK) {
		ESP_LOGE(tag, "setInterruptType: %d", rc);
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
	::gpio_set_level(pin, value);
} // write


