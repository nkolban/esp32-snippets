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
#include "GeneralUtils.h"

static const char* LOG_TAG = "I2C";

static bool driverInstalled = false;
static bool debug = false;
/**
 * @brief Create an instance of an %I2C object.
 * @return N/A.
 */
I2C::I2C() {
	m_directionKnown = false;
	m_address = 0;
	m_cmd     = 0;
	m_sdaPin  = DEFAULT_SDA_PIN;
	m_sclPin  = DEFAULT_CLK_PIN;
	m_portNum = I2C_NUM_0;
} // I2C


/**
 * @brief Begin a new %I2C transaction.
 *
 * Begin a transaction by adding an %I2C start to the queue.
 * @return N/A.
 */
void I2C::beginTransaction() {
	if (debug) {
		ESP_LOGD(LOG_TAG, "beginTransaction()");
	}
	m_cmd = ::i2c_cmd_link_create();
	esp_err_t errRc = ::i2c_master_start(m_cmd);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "i2c_master_start: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
	}
	m_directionKnown = false;
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
		ESP_LOGD(LOG_TAG, "endTransaction()");
	}
	esp_err_t errRc = ::i2c_master_stop(m_cmd);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "i2c_master_stop: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
	}

	errRc = ::i2c_master_cmd_begin(m_portNum, m_cmd, 1000 / portTICK_PERIOD_MS);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "i2c_master_cmd_begin: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
	}
	::i2c_cmd_link_delete(m_cmd);
	m_directionKnown = false;
} // endTransaction


/**
 * @brief Get the address of the %I2C slave against which we are working.
 *
 * @return The address of the %I2C slave.
 */
uint8_t I2C::getAddress() const {
	return m_address;
}


/**
 * @brief Initialize the I2C interface.
 *
 * @param [in] address The address of the slave device.
 * @param [in] sdaPin The pin to use for SDA data.
 * @param [in] sclPin The pin to use for SCL clock.
 * @return N/A.
 */
void I2C::init(uint8_t address, gpio_num_t sdaPin, gpio_num_t sclPin, uint32_t clockSpeed, i2c_port_t portNum, bool pullup) {
	ESP_LOGD(LOG_TAG, ">> I2c::init.  address=%d, sda=%d, scl=%d, clockSpeed=%d, portNum=%d", address, sdaPin, sclPin, clockSpeed, portNum);
	assert(portNum < I2C_NUM_MAX);
	m_portNum = portNum;
	m_sdaPin  = sdaPin;
	m_sclPin  = sclPin;
	m_address = address;

	i2c_config_t conf;
	conf.mode             = I2C_MODE_MASTER;
	conf.sda_io_num       = sdaPin;
	conf.scl_io_num       = sclPin;
	conf.sda_pullup_en    = pullup ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
	conf.scl_pullup_en    = pullup ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
	conf.master.clk_speed =  clockSpeed;
	esp_err_t errRc = ::i2c_param_config(m_portNum, &conf);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "i2c_param_config: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
	}
	if (!driverInstalled) {
		errRc = ::i2c_driver_install(m_portNum, I2C_MODE_MASTER, 0, 0, 0);
		if (errRc != ESP_OK) {
			ESP_LOGE(LOG_TAG, "i2c_driver_install: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		}
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
		ESP_LOGD(LOG_TAG, "read(size=%d, ack=%d)", length, ack);
	}
	if (!m_directionKnown) {
		m_directionKnown = true;
		esp_err_t errRc = ::i2c_master_write_byte(m_cmd, (m_address << 1) | I2C_MASTER_READ, !ack);
		if (errRc != ESP_OK) {
			ESP_LOGE(LOG_TAG, "i2c_master_write_byte: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		}
	}
	esp_err_t errRc = ::i2c_master_read(m_cmd, bytes, length, ack?I2C_MASTER_ACK:I2C_MASTER_NACK);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "i2c_master_read: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
	}
} // read


/**
 * @brief Read a single byte from the slave.
 *
 * @param [out] byte The address into which the read byte will be stored.
 * @param [in] ack Whether or not we should send an ACK to the slave after reading a byte.
 * @return N/A.
 */
void I2C::read(uint8_t* byte, bool ack) {
	if (debug) {
		ESP_LOGD(LOG_TAG, "read(size=1, ack=%d)", ack);
	}
	if (!m_directionKnown) {
		m_directionKnown = true;
		esp_err_t errRc = ::i2c_master_write_byte(m_cmd, (m_address << 1) | I2C_MASTER_READ, !ack);
		if (errRc != ESP_OK) {
			ESP_LOGE(LOG_TAG, "i2c_master_write_byte: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		}
	}
	ESP_ERROR_CHECK(::i2c_master_read_byte(m_cmd, byte, ack?I2C_MASTER_ACK:I2C_MASTER_NACK));
} // readByte


/**
 * @brief Scan the I2C bus looking for devices.
 *
 * A scan is performed on the I2C bus looking for devices.  A table is written
 * to the serial output describing what devices (if any) were found.
 * @return N/A.
 */
void I2C::scan() {
	printf("Data Pin: %d, Clock Pin: %d\n", this->m_sdaPin, this->m_sclPin);
	printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
	printf("00:         ");
	for (uint8_t i = 3; i < 0x78; i++) {
		if (i % 16 == 0) {
			printf("\n%.2x:", i);
		}
		if (slavePresent(i)) {
			printf(" %.2x", i);
		} else {
			printf(" --");
		}
	}
	printf("\n");
} // scan


/**
 * @brief Set the address of the %I2C slave against which we will be working.
 *
 * @param [in] address The address of the %I2C slave.
 */
void I2C::setAddress(uint8_t address) {
	this->m_address = address;
} // setAddress


/**
 * @brief enable or disable debugging.
 * @param [in] enabled Should debugging be enabled or disabled.
 * @return N/A.
 */
void I2C::setDebug(bool enabled) {
	debug = enabled;
} // setDebug


/**
 * @brief Determine if the slave is present and responding.
 * @param [in] The address of the slave.
 * @return True if the slave is present and false otherwise.
 */
bool I2C::slavePresent(uint8_t address) {
	i2c_cmd_handle_t cmd = ::i2c_cmd_link_create();
	::i2c_master_start(cmd);
	::i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
	::i2c_master_stop(cmd);

	esp_err_t espRc = ::i2c_master_cmd_begin(m_portNum, cmd, 100 / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);
	return espRc == 0;  // Return true if the slave is present and false otherwise.
} // slavePresent


/**
 * @brief Add an %I2C start request to the command stream.
 * @return N/A.
 */
void I2C::start() {
	if (debug) {
		ESP_LOGD(LOG_TAG, "start()");
	}
	esp_err_t errRc = ::i2c_master_start(m_cmd);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "i2c_master_start: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
	}
	m_directionKnown = false;
} // start


/**
 * @brief Add an %I2C stop request to the command stream.
 * @return N/A.
 */
void I2C::stop() {
	if (debug) {
		ESP_LOGD(LOG_TAG, "stop()");
	}
	esp_err_t errRc = ::i2c_master_stop(m_cmd);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "i2c_master_stop: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
	}
	m_directionKnown = false;
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
		ESP_LOGD(LOG_TAG, "write(val=0x%.2x, ack=%d)", byte, ack);
	}
	if (!m_directionKnown) {
		m_directionKnown = true;
		esp_err_t errRc = ::i2c_master_write_byte(m_cmd, (m_address << 1) | I2C_MASTER_WRITE, !ack);
		if (errRc != ESP_OK) {
			ESP_LOGE(LOG_TAG, "i2c_master_write_byte: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		}
	}
	esp_err_t errRc = ::i2c_master_write_byte(m_cmd, byte, !ack);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "i2c_master_write_byte: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
	}
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
		ESP_LOGD(LOG_TAG, "write(length=%d, ack=%d)", length, ack);
	}
	if (!m_directionKnown) {
		m_directionKnown = true;
		esp_err_t errRc = ::i2c_master_write_byte(m_cmd, (m_address << 1) | I2C_MASTER_WRITE, !ack);
		if (errRc != ESP_OK) {
			ESP_LOGE(LOG_TAG, "i2c_master_write_byte: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		}
	}
	esp_err_t errRc = ::i2c_master_write(m_cmd, bytes, length, !ack);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "i2c_master_write: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
	}
} // write
