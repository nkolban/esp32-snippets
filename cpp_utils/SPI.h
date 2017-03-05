/*
 * SPI.h
 *
 *  Created on: Mar 3, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_SPI_H_
#define COMPONENTS_CPP_UTILS_SPI_H_
#include <driver/spi_master.h>
/**
 * @brief Handle SPI protocol.
 */
class SPI {
public:
	SPI(int mosiPin=13, int misoPin=12, int clkPin=14, int csPin=15);
	virtual ~SPI();
	void transfer(uint8_t *data, size_t dataLen);
private:
  spi_device_handle_t handle;
};

#endif /* COMPONENTS_CPP_UTILS_SPI_H_ */
