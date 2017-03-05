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
GPIO::GPIO() {
	// TODO Auto-generated constructor stub

}

/**
 * @brief Determine if the pin is a valid pin for an ESP32 (i.e. is it in range).
 *
 * @param [in] pin The pin number to validate.
 * @return The value of true if the pin is valid and false otherwise.
 */
bool GPIO::inRange(gpio_num_t pin) {
	if (pin>=0 && pin<=39) {
		return true;
	}
	return false;
}

/**
 * @brief Read a value from the given pin.
 * @param [in] pin The pin to read from.
 */
bool GPIO::read(gpio_num_t pin) {
	return ::gpio_get_level(pin);
}


/**
 * @brief Write a value to the given pin.
 *
 * @param [in] pin The gpio pin to change.
 * @param [out] value The value to be written to the pin.
 */
void GPIO::write(gpio_num_t pin, bool value) {
	::gpio_set_level(pin, value);
}



