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
public:
	/**
	 * @brief The default SDA pin.
	 */
	static const gpio_num_t DEFAULT_SDA_PIN = GPIO_NUM_25;

	/**
	 * @brief The default Clock pin.
	 */
	static const gpio_num_t DEFAULT_CLK_PIN = GPIO_NUM_26;

	/**
	 * @brief The default Clock speed.
	 */
	static const uint32_t DEFAULT_CLK_SPEED = 100000;

	I2C();
	void beginTransaction();
	void endTransaction();
	uint8_t getAddress() const;
	void init(uint8_t address, gpio_num_t sdaPin = DEFAULT_SDA_PIN, gpio_num_t sclPin = DEFAULT_CLK_PIN, uint32_t clkSpeed = DEFAULT_CLK_SPEED, i2c_port_t portNum = I2C_NUM_0, bool pullup = true);
	void read(uint8_t* bytes, size_t length, bool ack = true);
	void read(uint8_t* byte, bool ack = true);
	void scan();
	void setAddress(uint8_t address);
	void setDebug(bool enabled);
	bool slavePresent(uint8_t address);
	void start();
	void stop();
	void write(uint8_t byte, bool ack = true);
	void write(uint8_t* bytes, size_t length, bool ack = true);

private:
	uint8_t		  m_address;
	i2c_cmd_handle_t m_cmd;
	bool			 m_directionKnown;
	gpio_num_t	   m_sdaPin;
	gpio_num_t	   m_sclPin;
	i2c_port_t	   m_portNum;

};

#endif /* MAIN_I2C_H_ */
