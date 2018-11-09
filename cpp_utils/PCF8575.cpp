/*
 * PCF8575.cpp
 *
 *  Created on: Jan 27, 2018
 *      Author: kolban
 */

#include "PCF8575.h"
#include "I2C.h"


/**
 * @brief Class constructor.
 *
 * The address is the address of the device on the %I2C bus.  This is the value 0x20 plus
 * the value of the device input pins `A0`, `A1` and `A2`.  This means that the address should
 * be between 0x20 and 0x27.
 *
 * @param [in] address The %I2C address of the device on the %I2C bus.
 */
PCF8575::PCF8575(uint8_t address) {
	i2c = new I2C();
	i2c->setAddress(address);
	m_lastWrite = 0;
}


/**
 * @brief Class instance destructor.
 */
PCF8575::~PCF8575() {
	delete i2c;
}


/**
 * @brief Read all the input bits from the device.
 * @return A 16 bit value representing the values on each of the input pins.
 */
uint16_t PCF8575::read() {
	uint16_t value;
	i2c->beginTransaction();
	i2c->read((uint8_t*) &value, true);
	i2c->read(((uint8_t*) &value) + 1, true);
	i2c->endTransaction();
	return value;
} // read


/**
 * @brief Read the logic level on a given pin.
 *
 * @param [in] bit The input pin of the device to read.  Values are 0-15.
 * @return True if the pin is high, false otherwise.  Undefined if there is no signal on the pin.
 */
bool PCF8575::readBit(uint16_t bit) {
	if (bit > 7) return false;
	uint16_t value = read();
	return (value & (1 << bit)) != 0;
} // readBit


/**
 * @brief Set the output values of the device.
 *
 * @param [in] value The bit pattern to set on the output.
 */
void PCF8575::write(uint16_t value) {
	if (invert) {
		value = ~value;
	}
	i2c->beginTransaction();
	i2c->write(value & 0xff, true);
	i2c->write((value >> 8) & 0xff, true);
	i2c->endTransaction();
	m_lastWrite = value;
} // write


/**
 * @brief Change the output value of a specific pin.
 *
 * The other bits beyond the one setting retain their values from the previous call to write() or
 * previous calls to writeBit().
 *
 * @param [in] bit The pin to have its value changed.  The pin may be 0-15.
 * @param [in] value The logic level to appear on the identified output pin.
 */
void PCF8575::writeBit(uint16_t bit, bool value) {
	if (bit > 15) return;
	if (invert) {
		value = !value;
	}
	if (value) {
		m_lastWrite |= (1 << bit);
	} else {
		m_lastWrite &= ~(1 << bit);
	}
	write(m_lastWrite);
} // writeBit


/**
 * @brief Invert the bit values.
 * Normally setting a pin's value to 1 means that a high signal is generated and a 0 means a low
 * signal is generated.  Setting the inversion to true, inverts that meaning.
 *
 * @param [in] value True if we wish to invert the signals and false otherwise.
 */
void PCF8575::setInvert(bool value) {
	this->invert = value;
} // setInvert


/**
 * @brief Initialize the PCF8575 device.
 *
 * @param [in] sdaPin The pin to use for the %I2C SDA functions.
 * @param [in] clkPin The pin to use for the %I2C CLK functions.
 */
void PCF8575::init(gpio_num_t sdaPin, gpio_num_t clkPin) {
	i2c->init(0, sdaPin, clkPin);
} // init
