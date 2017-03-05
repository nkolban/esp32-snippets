/*
 * GPIO.h
 *
 *  Created on: Feb 28, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_GPIO_H_
#define COMPONENTS_CPP_UTILS_GPIO_H_
#include <driver/gpio.h>
/**
 * @brief Interface to GPIO functions.
 */
class GPIO {
public:
	GPIO();
	static bool inRange(gpio_num_t pin);
	static bool read(gpio_num_t pin);
	static void write(gpio_num_t pin, bool value);

};

#endif /* COMPONENTS_CPP_UTILS_GPIO_H_ */
