/*
 * GPIO.cpp
 *
 *  Created on: Feb 28, 2017
 *      Author: kolban
 */

#include "GPIO.h"

GPIO::GPIO() {
	// TODO Auto-generated constructor stub

}

/**
 * Determine if the pin is a valid pin for an ESP32 (i.e. is it in range).
 * @param [in] pin The pin number to validate.
 * @return The value of true if the pin is valid and false otherwise.
 */
bool GPIO::inRange(int pin) {
	if (pin>=0 && pin<=39) {
		return true;
	}
	return false;
}
