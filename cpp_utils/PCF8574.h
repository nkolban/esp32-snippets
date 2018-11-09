/*
 * PCF8574.h
 *
 *  Created on: Mar 2, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_PCF8574_H_
#define COMPONENTS_CPP_UTILS_PCF8574_H_
#include "I2C.h"

/**
 * @brief Encapsulate a %PCF8574 device.
 *
 * The %PCF8574 is a 8 bit %GPIO expander attached to %I2C.  It can read and write 8 bits of data
 * and hence has 8 pins that can be used for input or output.
 *
 * @see [PCF8574 home page](http://www.ti.com/product/PCF8574)
 */
class PCF8574 {
public:
	PCF8574(uint8_t address);
	virtual ~PCF8574();
	void init(gpio_num_t sdaPin = I2C::DEFAULT_SDA_PIN, gpio_num_t clkPin = I2C::DEFAULT_CLK_PIN);
	uint8_t read();
	bool readBit(uint8_t bit);
	void setInvert(bool value);
	void write(uint8_t value);
	void writeBit(uint8_t bit, bool value);

private:
	I2C* i2c;
	uint8_t lastWrite;
	bool invert = false;

};

#endif /* COMPONENTS_CPP_UTILS_PCF8574_H_ */
