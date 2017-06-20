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
#include <esp_log.h>

static char tag[] = "I2C";

static bool driverInstalled = false;
static bool debug = false;
/**
 * @brief Create an instance of an %I2C object.
 * @return N/A.
 */
I2C::I2C() {
	directionKnown = false;
	address = 0;
	cmd     = 0;
	sdaPin  = DEFAULT_SDA_PIN;
	sclPin  = DEFAULT_SDA_PIN;
} // I2C


/**
 * @brief Begin a new %I2C transaction.
 *
 * Begin a transaction by adding an %I2C start to the queue.
 * @return N/A.
 */
void I2C::beginTransaction() {
	if (debug) {
		ESP_LOGD(tag, "beginTransaction()");
	}
	cmd = ::i2c_cmd_link_create();
	ESP_ERROR_CHECK(::i2c_master_start(cmd));
	directionKnown = false;
} // beginTransaction


/**
 * @brief End an I2C transaction.
 *
 * This call will execute the %I2C requests that have been queued up since the preceding call to the
 * beginTransaction() function.  An %I2C stop() is also called.
 * @return N/A.
 */
void I2C::endTransaction() {
	if (debug) {
		ESP_LOGD(tag, "endTransaction()");
	}
	ESP_ERROR_CHECK(::i2c_master_stop(cmd));
	ESP_ERROR_CHECK(::i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS));
	::i2c_cmd_link_delete(cmd);
	directionKnown = false;
} // endTransaction


/**
 * @brief Initialize the I2C interface.
 *
 * @param [in] address The address of the slave device.
 * @param [in] sdaPin The pin to use for SDA data.
 * @param [in] sclPin The pin to use for SCL clock.
 * @return N/A.
 */
void I2C::init(uint8_t address, gpio_num_t sdaPin, gpio_num_t sclPin) {
	ESP_LOGD(tag, ">> I2c::init.  address=%d, sda=%d, scl=%d", address, sdaPin, sclPin);
	this->sdaPin = sdaPin;
	this->sclPin = sclPin;
	this->address = address;
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num       = sdaPin;
	conf.scl_io_num       = sclPin;
	conf.sda_pullup_en    = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en    = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 100000;
	ESP_ERROR_CHECK(::i2c_param_config(I2C_NUM_0, &conf));
	if (!driverInstalled) {
		ESP_ERROR_CHECK(::i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
		driverInstalled = true;
	}
} // init


/**
 * @brief Read a sequence of bytes from the slave.
 *
 * @param [out] bytes The address into which the read bytes will be stored.
 * @param [in] length The number of expected bytes to read.
 * @param [in] ack Whether or not we should send an ACK to the slave after reading a byte.
 * @return N/A.
 */
void I2C::read(uint8_t* bytes, size_t length, bool ack) {
	if (debug) {
		ESP_LOGD(tag, "read(size=%d, ack=%d)", length, ack);
	}
	if (directionKnown == false) {
		directionKnown = true;
		ESP_ERROR_CHECK(::i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_READ, !ack));
	}
	ESP_ERROR_CHECK(::i2c_master_read(cmd, bytes, length, !ack));
} // read


/**
 * @brief Read a single byte from the slave.
 *
 * @param [out] byte The address into which the read byte will be stored.
 * @param [in] ack Whether or not we should send an ACK to the slave after reading a byte.
 * @return N/A.
 */
void I2C::read(uint8_t *byte, bool ack) {
	if (debug) {
		ESP_LOGD(tag, "read(size=1, ack=%d)", ack);
	}
	if (directionKnown == false) {
		directionKnown = true;
		ESP_ERROR_CHECK(::i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_READ, !ack));
	}
	ESP_ERROR_CHECK(::i2c_master_read_byte(cmd, byte, !ack));
} // readByte


/**
 * @brief Scan the I2C bus looking for devices.
 *
 * A scan is performed on the I2C bus looking for devices.  A table is written
 * to the serial output describing what devices (if any) were found.
 * @return N/A.
 */
void I2C::scan() {
	int i;
	esp_err_t espRc;
	printf("Data Pin: %d, Clock Pin: %d\n", this->sdaPin, this->sclPin);
	printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
	printf("00:         ");
	for (i=3; i< 0x78; i++) {
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
		i2c_master_stop(cmd);

		espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100/portTICK_PERIOD_MS);
		if (i%16 == 0) {
			printf("\n%.2x:", i);
		}
		if (espRc == 0) {
			printf(" %.2x", i);
		} else {
			printf(" --");
		}
		//ESP_LOGD(tag, "i=%d, rc=%d (0x%x)", i, espRc, espRc);
		i2c_cmd_link_delete(cmd);
	}
	printf("\n");
} // scan


/**
 * @brief enable or disable debugging.
 * @param [in] enabled Should debugging be enabled or disabled.
 * @return N/A.
 */
void I2C::setDebug(bool enabled) {
	debug = enabled;
} // setDebug


/**
 * @brief Add an %I2C start request to the command stream.
 * @return N/A.
 */
void I2C::start() {
	if (debug) {
		ESP_LOGD(tag, "start()");
	}
	ESP_ERROR_CHECK(::i2c_master_start(cmd));
} // start


/**
 * @brief Add an %I2C stop request to the command stream.
 * @return N/A.
 */
void I2C::stop() {
	if (debug) {
		ESP_LOGD(tag, "stop()");
	}
	ESP_ERROR_CHECK(::i2c_master_stop(cmd));
	directionKnown = false;
} // stop


/**
 * @brief Write a single byte to the %I2C slave.
 *
 * @param[in] byte The byte to write to the slave.
 * @param[in] ack Whether or not an acknowledgment is expected from the slave.
 * @return N/A.
 */
void I2C::write(uint8_t byte, bool ack) {
	if (debug) {
		ESP_LOGD(tag, "write(val=0x%.2x, ack=%d)", byte, ack);
	}
	if (directionKnown == false) {
		directionKnown = true;
		ESP_ERROR_CHECK(::i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, !ack));
	}
	ESP_ERROR_CHECK(::i2c_master_write_byte(cmd, byte, !ack));
} // write


/**
 * @brief Write a sequence of byte to the %I2C slave.
 *
 * @param [in] bytes The sequence of bytes to write to the %I2C slave.
 * @param [in] length The number of bytes to write.
 * @param [in] ack Whether or not an acknowledgment is expected from the slave.
 * @return N/A.
 */
void I2C::write(uint8_t *bytes, size_t length, bool ack) {
	if (debug) {
		ESP_LOGD(tag, "write(length=%d, ack=%d)", length, ack);
	}
	if (directionKnown == false) {
		directionKnown = true;
		ESP_ERROR_CHECK(::i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, !ack));
	}
	ESP_ERROR_CHECK(::i2c_master_write(cmd, bytes, length, !ack));
} // write


