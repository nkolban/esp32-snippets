/*
 * I2C.cpp
 *
 *  Created on: Feb 24, 2017
 *      Author: kolban
 */
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <esp_err.h>
#include <stdint.h>
#include <sys/types.h>
#include "I2C.h"
#include "sdkconfig.h"


/**
 * Create an instance of an I2C object.
 * @param[in] sdaPin The pin number used for the SDA line.
 * @param[in] sclPin The pin number used for the SCL line.
 */
I2C::I2C(int sdaPin, int sclPin) {
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = (gpio_num_t)sdaPin;
	conf.scl_io_num = (gpio_num_t)sclPin;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 100000;
	ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
	ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
	directionKnown = false;
	address = 0;
	cmd = 0;
}


/**
 * Begin a new I2C transaction.
 */
void I2C::beginTransaction() {
	cmd = i2c_cmd_link_create();
	ESP_ERROR_CHECK(i2c_master_start(cmd));
	directionKnown = false;
}

/**
 * End an I2C transaction.
 * This call will execute the I2C requests that have been queued up since the preceding call to the
 * `beginTransaction()` function.
 */
void I2C::endTransaction() {
	ESP_ERROR_CHECK(i2c_master_stop(cmd));
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
}


/**
 * Write a single byte to the I2C slave.
 * @param[in] byte The byte to write to the slave.
 * @param[in] ack Whether or not an acknowledgement is expected from the slave.
 */
void I2C::write(uint8_t byte, bool ack) {
	if (directionKnown == false) {
		directionKnown = true;
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, 1));
	}
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, byte, ack));
}

/**
 * Write a sequence of byte to the I2C slave.
 * @param[in] bytes The sequence of bytes to write to the I2C slave.
 * @param[in] length The number of bytes to write.
 * @param[in] ack Whether or not an acknowledgement is expected from the slave.
 */
void I2C::write(uint8_t *bytes, size_t length, bool ack) {
	if (directionKnown == false) {
		directionKnown = true;
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, ack));
	}
	ESP_ERROR_CHECK(i2c_master_write(cmd, bytes, length, 1));
}


/**
 * Add an I2C start request to the command stream.
 */
void I2C::start() {
	ESP_ERROR_CHECK(i2c_master_start(cmd));
}

/**
 * Read a single byte from the slave.
 * @param[out] byte The address into which the read byte will be stored.
 * @param[in] ack Whether or not we should send an ACK to the slave after reading a byte.
 */
void I2C::readByte(uint8_t* byte, bool ack) {
	if (directionKnown == false) {
		directionKnown = true;
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_READ, 1));
	}
	ESP_ERROR_CHECK(i2c_master_read_byte(cmd, byte, !ack));
}

/**
 * Read a sequence of bytes from the slave.
 * @param[out] byte The address into which the read bytes will be stored.
 * @param[in] length The number of expected bytes to read.
 * @param[in] ack Whether or not we should send an ACK to the slave after reading a byte.
 */
void I2C::read(uint8_t* bytes, size_t length, bool ack) {
	if (directionKnown == false) {
		directionKnown = true;
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_READ, 1));
	}
	ESP_ERROR_CHECK(i2c_master_read(cmd, bytes, length, !ack));
}


/**
 * Add an I2C stop request to the command stream.
 */
void I2C::stop() {
	ESP_ERROR_CHECK(i2c_master_stop(cmd));
}
