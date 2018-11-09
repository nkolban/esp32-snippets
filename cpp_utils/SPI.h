/*
 * SPI.h
 *
 *  Created on: Mar 3, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_SPI_H_
#define COMPONENTS_CPP_UTILS_SPI_H_
#include <driver/spi_master.h>
#include <driver/gpio.h>
/**
 * @brief Handle %SPI protocol.
 */
class SPI {
public:
	SPI();
	virtual ~SPI();
	void    init(
		int mosiPin = DEFAULT_MOSI_PIN,
		int misoPin = DEFAULT_MISO_PIN,
		int clkPin  = DEFAULT_CLK_PIN,
		int csPin   = DEFAULT_CS_PIN);
	void    setHost(spi_host_device_t host);
	void    transfer(uint8_t* data, size_t dataLen);
	uint8_t transferByte(uint8_t value);

	/**
	 * @brief The default MOSI pin.
	 */
	static const int DEFAULT_MOSI_PIN = GPIO_NUM_13;

	/**
	 * @brief The default MISO pin.
	 */
	static const int DEFAULT_MISO_PIN = GPIO_NUM_12;

	/**
	 * @brief The default CLK pin.
	 */
	static const int DEFAULT_CLK_PIN  = GPIO_NUM_14;

	/**
	 * @brief The default CS pin.
	 */
	static const int DEFAULT_CS_PIN   = GPIO_NUM_15;

	/**
	 * @brief Value of unset pin.
	 */
	static const int PIN_NOT_SET	  = -1;

private:
  spi_device_handle_t m_handle;
  spi_host_device_t   m_host;

};

#endif /* COMPONENTS_CPP_UTILS_SPI_H_ */
