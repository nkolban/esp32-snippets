/*
 * SmartLED.h
 *
 *  Created on: Oct 22, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_SMARTLED_H_
#define COMPONENTS_SMARTLED_H_
#include <stdint.h>
/**
 * @brief A data type representing the color of a pixel.
 */
typedef struct {
	/**
	 * @brief The red component of the pixel.
	 */
	uint8_t red;
	/**
	 * @brief The green component of the pixel.
	 */
	uint8_t green;
	/**
	 * @brief The blue component of the pixel.
	 */
	uint8_t blue;
} pixel_t;


class SmartLED {
public:
	SmartLED();
	virtual ~SmartLED();
	uint32_t getBrightness();
	uint16_t getPixelCount();
	virtual void init() = 0;
	virtual void show() = 0;
	void setBrightness(uint32_t percent);
	void setColorOrder(char* order);
	void setPixel(uint16_t index, uint8_t red, uint8_t green, uint8_t blue);
	void setPixel(uint16_t index, pixel_t pixel);
	void setPixel(uint16_t index, uint32_t pixel);
	void setPixelCount(uint16_t pixelCount);
	void setHSBPixel(uint16_t index, uint16_t hue, uint8_t saturation, uint8_t brightness);
	void clear();

protected:
	uint32_t       m_brightness;
	char*          m_colorOrder;
	uint16_t       m_pixelCount;
	pixel_t*       m_pixels;
};

#endif /* COMPONENTS_SMARTLED_H_ */
