/*
 * GPIO.cpp
 *
 *  Created on: Feb 28, 2017
 *      Author: kolban
 */

#include "GPIO.h"
#include <driver/gpio.h>

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

