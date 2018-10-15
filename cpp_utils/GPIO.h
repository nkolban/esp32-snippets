/*
 * GPIO.h
 *
 *  Created on: Feb 28, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_GPIO_H_
#define COMPONENTS_CPP_UTILS_GPIO_H_
#include <driver/gpio.h>
namespace ESP32CPP {
	/**
	 * @brief Interface to %GPIO functions.
	 *
	 * The %GPIO functions encapsulate the %GPIO access.  The GPIOs available to us are
	 * 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,21,23,25,26,27,32,33,34,35,36,37,38,39.
	 *
	 * The GPIOs of 34,35,36,37,38 and 39 are input only.
	 *
	 * Note that we must not use `int` values for the pin numbers but instead use the `gpio_num_t`.  There
	 * are constants defined for these of the form `GPIO_NUM_xx`.
	 *
	 * To toggle a pin we might code:
	 *
	 * @code{.cpp}
	 * ESP32CPP::GPIO::setOutput(pin);
	 * ESP32CPP::GPIO::high(pin);
	 * ESP32CPP::GPIO::low(pin);
	 * @endcode
	 */
	class GPIO {
	public:
		static void addISRHandler(gpio_num_t pin, gpio_isr_t handler, void* pArgs);
		static void high(gpio_num_t pin);
		static void interruptDisable(gpio_num_t pin);
		static void interruptEnable(gpio_num_t pin);
		static bool inRange(gpio_num_t pin);
		static void low(gpio_num_t pin);
		static bool read(gpio_num_t pin);
		static void setInput(gpio_num_t pin);
		static void setInterruptType(gpio_num_t pin, gpio_int_type_t intrType);
		static void setOutput(gpio_num_t pin);
		static void write(gpio_num_t pin, bool value);
		static void writeByte(gpio_num_t pins[], uint8_t value, int bits);
	}; // End GPIO
} // End ESP32CPP namespace
#endif /* COMPONENTS_CPP_UTILS_GPIO_H_ */
