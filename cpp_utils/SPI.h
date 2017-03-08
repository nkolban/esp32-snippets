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
 * @brief Handle SPI protocol.
 */
class SPI {
public:
	SPI();
	virtual ~SPI();
	void init(gpio_num_t mosiPin=GPIO_NUM_13, gpio_num_t misoPin=GPIO_NUM_12, gpio_num_t clkPin=GPIO_NUM_14, gpio_num_t csPin=GPIO_NUM_15);
	void transfer(uint8_t *data, size_t dataLen);
private:
  spi_device_handle_t handle;
};

#endif /* COMPONENTS_CPP_UTILS_SPI_H_ */
