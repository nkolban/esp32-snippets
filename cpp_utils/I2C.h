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
#include <driver/gpio.h>


/**
 * @brief Interface to %I2C functions.
 */
class I2C {
private:
	uint8_t address;
	i2c_cmd_handle_t cmd;
	bool directionKnown;
	gpio_num_t sdaPin;
	gpio_num_t sclPin;

public:
	I2C();
	void beginTransaction();
	void endTransaction();
	/**
	 * @brief Get the address of the %I2C slave against which we are working.
	 *
	 * @return The address of the %I2C slave.
	 */
	uint8_t getAddress() const
	{
		return address;
	}

	void init(uint8_t address, gpio_num_t sdaPin = DEFAULT_SDA_PIN, gpio_num_t sclPin = DEFAULT_CLK_PIN);
	void read(uint8_t *bytes, size_t length, bool ack=true);
	void read(uint8_t *byte, bool ack=true);

	/**
	 * @brief Set the address of the %I2C slave against which we will be working.
	 *
	 * @param[in] address The address of the %I2C slave.
	 */
	void setAddress(uint8_t address)
	{
		this->address = address;
	}

	void setDebug(bool enabled);
	void scan();
	void start();
	void stop();
	void write(uint8_t byte, bool ack=true);
	void write(uint8_t *bytes, size_t length, bool ack=true);
	/**
	 * @brief The default SDA pin.
	 */
	static const gpio_num_t DEFAULT_SDA_PIN = GPIO_NUM_25;
	/**
	 * @brief The default Clock pin.
	 */
	static const gpio_num_t DEFAULT_CLK_PIN = GPIO_NUM_26;
};

#endif /* MAIN_I2C_H_ */
