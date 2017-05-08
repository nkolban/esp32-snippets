/*
 * GPIO.h
 *
 *  Created on: Feb 28, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_GPIO_H_
#define COMPONENTS_CPP_UTILS_GPIO_H_
#include <driver/gpio.h>
namespace ESP32CPP
{
/**
 * @brief Interface to %GPIO functions.
 */
class GPIO {
public:
	//GPIO();
	/**
	 * @brief Set the pin high.
	 * @param [in] pin The pin to be set high.
	 */
	static void high(gpio_num_t pin) {
		write(pin, true);
	}

	static bool inRange(gpio_num_t pin);
	/**
	 * @brief Set the pin low.
	 * @param [in] pin The pin to be set low.
	 */
	static void low(gpio_num_t pin) {
		write(pin, false);
	}
	static bool read(gpio_num_t pin);
	static void setInput(gpio_num_t pin);
	static void setOutput(gpio_num_t pin);
	static void write(gpio_num_t pin, bool value);


};
}
#endif /* COMPONENTS_CPP_UTILS_GPIO_H_ */
