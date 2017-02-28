/*
 * I2C.h
 *
 *  Created on: Feb 24, 2017
 *      Author: kolban
 */

#ifndef MAIN_I2C_H_
#define MAIN_I2C_H_
#include <stdint.h>
#include <sys/types.h>
#include <driver/i2c.h>

class I2C {
private:
	uint8_t address;
	i2c_cmd_handle_t cmd;
	bool directionKnown;

public:
	I2C(int sdaPin, int sclPin);
	void beginTransaction();
	void endTransaction();
	void write(uint8_t byte, bool ack=true);
	void write(uint8_t *bytes, size_t length, bool ack=true);
	void readByte(uint8_t *byte, bool ack=true);
	void read(uint8_t *bytes, size_t length, bool ack=true);
	void start();
	void stop();

	/**
	 * Get the address of the I2C slave against which we are working.
	 * @return The address of the I2C slave.
	 */
	uint8_t getAddress() const
	{
		return address;
	}

	/**
	 * Set the address of the I2C slave against which we will be working.
	 * @param[in] address The address of the I2C slave.
	 */
	void setAddress(uint8_t address)
	{
		this->address = address;
	}
};

#endif /* MAIN_I2C_H_ */
