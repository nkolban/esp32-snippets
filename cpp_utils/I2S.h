/*
 * I2S.h
 *
 *  Created on: Jul 23, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_I2S_H_
#define COMPONENTS_CPP_UTILS_I2S_H_
#include <driver/gpio.h>
#include <FreeRTOS.h>
typedef struct {
    gpio_num_t pin_reset;          /*!< GPIO pin for camera reset line - OUT */
    gpio_num_t pin_xclk;           /*!< GPIO pin for camera XCLK line  - IN  */
    gpio_num_t pin_sscb_sda;       /*!< GPIO pin for camera SDA line   - OUT */
    gpio_num_t pin_sscb_scl;       /*!< GPIO pin for camera SCL line   - OUT */
    gpio_num_t pin_d7;             /*!< GPIO pin for camera D7 line    - IN  */
    gpio_num_t pin_d6;             /*!< GPIO pin for camera D6 line    - IN  */
    gpio_num_t pin_d5;             /*!< GPIO pin for camera D5 line    - IN  */
    gpio_num_t pin_d4;             /*!< GPIO pin for camera D4 line    - IN  */
    gpio_num_t pin_d3;             /*!< GPIO pin for camera D3 line    - IN  */
    gpio_num_t pin_d2;             /*!< GPIO pin for camera D2 line    - IN  */
    gpio_num_t pin_d1;             /*!< GPIO pin for camera D1 line    - IN  */
    gpio_num_t pin_d0;             /*!< GPIO pin for camera D0 line    - IN  */
    gpio_num_t pin_vsync;          /*!< GPIO pin for camera VSYNC line - IN  */
    gpio_num_t pin_href;           /*!< GPIO pin for camera HREF line  - IN  */
    gpio_num_t pin_pclk;           /*!< GPIO pin for camera PCLK line  - IN  */

    int xclk_freq_hz;       /*!< Frequency of XCLK signal, in Hz */
} dma_config_t;

class DMAData {
public:
	uint32_t getLength() {
		return m_length;
	}
	uint8_t* getData() {
		return m_pData;
	}

	void free() {
		delete[] m_pData;
		m_pData = nullptr;
	}
private:
	friend class I2S;
	uint32_t m_length;
	uint8_t* m_pData;
};

class I2S {
public:
	I2S();
	virtual ~I2S();
	void cameraMode(dma_config_t config, int desc_count, int sample_count);
	DMAData waitForData();
	FreeRTOS::Semaphore m_dmaSemaphore;
private:

};

#endif /* COMPONENTS_CPP_UTILS_I2S_H_ */
