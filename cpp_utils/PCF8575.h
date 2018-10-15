/*
 * PCF8575.h
 *
 *  Created on: Jan 27, 2018
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_PCF8575_H_
#define COMPONENTS_CPP_UTILS_PCF8575_H_
#include "I2C.h"

/**
 * @brief Encapsulate a %PCF8575 device.
 *
 * The %PCF8575 is a 16 bit %GPIO expander attached to %I2C.  It can read and write 16 bits of data
 * and hence has 16 pins that can be used for input or output.
 *
 * @see [PCF8575 home page](http://www.ti.com/product/PCF8575)
 */
class PCF8575 {
public:
	PCF8575(uint8_t address);
	virtual ~PCF8575();
	void     init(gpio_num_t sdaPin = I2C::DEFAULT_SDA_PIN, gpio_num_t clkPin = I2C::DEFAULT_CLK_PIN);
	uint16_t read();
	bool     readBit(uint16_t bit);
	void     setInvert(bool value);
	void     write(uint16_t value);
	void     writeBit(uint16_t bit, bool value);

private:
	I2C* i2c;
	uint16_t m_lastWrite;
	bool    invert = false;
};

#endif /* COMPONENTS_CPP_UTILS_PCF8575_H_ */
