/*
 * SmartLED.cpp
 *
 *  Created on: Oct 22, 2017
 *      Author: kolban
 */

#include "SmartLED.h"
#include "string.h"
#include <esp_log.h>

static const char* LOG_TAG = "SmartLED";

SmartLED::SmartLED() {
	m_brightness = 100;
	m_pixelCount = 0;
	m_pixels = nullptr;
	m_colorOrder = (char*) "GRB";
} // SmartLED


SmartLED::~SmartLED() {
	if (m_pixels != nullptr) {
		delete[] m_pixels;   // Delete the allocated storage for the pixels.
	}
} // ~SmartLED

/**
 * @brief Clear all the pixel colors.
 *
 * This sets all the pixels to off which is no brightness for all of the color channels.
 * The LEDs are not actually updated until a call to show() is subsequently made.
 */
void SmartLED::clear() {
	for (auto i = 0; i < this->m_pixelCount; i++) {
		m_pixels[i].red   = 0;
		m_pixels[i].green = 0;
		m_pixels[i].blue  = 0;
	} // End loop over all the pixel
} // clear


/**
 * @brief Get the brightness as a percentage.
 * @return The brightness as a percentage.
 */
uint32_t SmartLED::getBrightness() {
	return m_brightness;
} // getBrightness


/**
 * @brief Return the number of pixels in the chain.
 * @return The number of pixels in the chain as previously set by setPixelCount().
 */
uint16_t SmartLED::getPixelCount() {
	return m_pixelCount;
} // getPixelCount


/**
 * @brief Set the brightness as a percentage.
 * @param [in] percent The brightness.
 */
void SmartLED::setBrightness(uint32_t percent) {
	m_brightness = percent;
} // setBrightness


/**
 * @brief Set the color order of data sent to the LEDs.
 *
 * Data is sent to the WS2812s in a serial fashion.  There are 8 bits of data for each of the three
 * channel colors (red, green and blue).  The WS2812 LEDs typically expect the data to arrive in the
 * order of "green" then "red" then "blue".  However, this has been found to vary between some
 * models and manufacturers.  What this means is that some want "red", "green", "blue" and still others
 * have their own orders.  This function can be called to override the default ordering of "GRB".
 * We can specify
 * an alternate order by supply an alternate three character string made up of 'R', 'G' and 'B'
 * for example "RGB".
 */
void SmartLED::setColorOrder(char* colorOrder) {
	if (colorOrder != nullptr && strlen(colorOrder) == 3) {
		m_colorOrder = colorOrder;
	}
} // setColorOrder


/**
 * @brief Set the given pixel to the specified color.
 *
 * The LEDs are not actually updated until a call to show() is subsequently made.
 *
 * @param [in] index The pixel that is to have its color set.
 * @param [in] red The amount of red in the pixel.
 * @param [in] green The amount of green in the pixel.
 * @param [in] blue The amount of blue in the pixel.
 */
void SmartLED::setPixel(uint16_t index, uint8_t red, uint8_t green, uint8_t blue) {
	//assert(index < m_pixelCount);
	m_pixels[index].red   = red;
	m_pixels[index].green = green;
	m_pixels[index].blue  = blue;
} // setPixel


/**
 * @brief Set the given pixel to the specified color.
 *
 * The LEDs are not actually updated until a call to show().
 *
 * @param [in] index The pixel that is to have its color set.
 * @param [in] pixel The color value of the pixel.
 */
void SmartLED::setPixel(uint16_t index, pixel_t pixel) {
	//assert(index < m_pixelCount);
	m_pixels[index] = pixel;
} // setPixel


/**
 * @brief Set the given pixel to the specified color.
 *
 * The LEDs are not actually updated until a call to show().
 *
 * @param [in] index The pixel that is to have its color set.
 * @param [in] pixel The color value of the pixel.
 */
void SmartLED::setPixel(uint16_t index, uint32_t pixel) {
	//assert(index < m_pixelCount);
	m_pixels[index].red   = pixel & 0xff;
	m_pixels[index].green = (pixel & 0xff00) >> 8;
	m_pixels[index].blue  = (pixel & 0xff0000) >> 16;
} // setPixel


void SmartLED::setPixelCount(uint16_t pixelCount) {
	ESP_LOGD(LOG_TAG, ">> setPixelCount: %d", pixelCount);
	if (m_pixels != nullptr) {
		delete[] m_pixels;
	}
	m_pixelCount = pixelCount;
	m_pixels     = new pixel_t[pixelCount];   // Allocate the storage for the pixels.
	ESP_LOGD(LOG_TAG, "<< setPixelCount");
}


/**
 * @brief Set the given pixel to the specified HSB color.
 *
 * The LEDs are not actually updated until a call to show().
 *
 * @param [in] index The pixel that is to have its color set.
 * @param [in] hue The amount of hue in the pixel (0-360).
 * @param [in] saturation The amount of saturation in the pixel (0-255).
 * @param [in] brightness The amount of brightness in the pixel (0-255).
 */
void SmartLED::setHSBPixel(uint16_t index, uint16_t hue, uint8_t saturation, uint8_t brightness) {
	double sat_red;
	double sat_green;
	double sat_blue;
	double ctmp_red;
	double ctmp_green;
	double ctmp_blue;
	double new_red;
	double new_green;
	double new_blue;
	double dSaturation=(double) saturation / 255;
	double dBrightness=(double) brightness / 255;

	//assert(index < pixelCount);

	if (hue < 120) {
		sat_red   = (120 - hue) / 60.0;
		sat_green = hue / 60.0;
		sat_blue  = 0;
	} else if (hue < 240) {
		sat_red   = 0;
		sat_green = (240 - hue) / 60.0;
		sat_blue  = (hue - 120) / 60.0;
	} else {
		sat_red   = (hue - 240) / 60.0;
		sat_green = 0;
		sat_blue  = (360 - hue) / 60.0;
	}

	if (sat_red>1.0) {
		sat_red = 1.0;
	}
	if (sat_green>1.0) {
		sat_green = 1.0;
	}
	if (sat_blue>1.0) {
		sat_blue = 1.0;
	}

	ctmp_red   = 2 * dSaturation * sat_red   + (1 - dSaturation);
	ctmp_green = 2 * dSaturation * sat_green + (1 - dSaturation);
	ctmp_blue  = 2 * dSaturation * sat_blue  + (1 - dSaturation);

	if (dBrightness < 0.5) {
		new_red   = dBrightness * ctmp_red;
		new_green = dBrightness * ctmp_green;
		new_blue  = dBrightness * ctmp_blue;
	} else {
		new_red   = (1 - dBrightness) * ctmp_red   + 2 * dBrightness - 1;
		new_green = (1 - dBrightness) * ctmp_green + 2 * dBrightness - 1;
		new_blue  = (1 - dBrightness) * ctmp_blue  + 2 * dBrightness - 1;
	}

	m_pixels[index].red   = (uint8_t)(new_red * 255);
	m_pixels[index].green = (uint8_t)(new_green * 255);
	m_pixels[index].blue  = (uint8_t)(new_blue * 255);
} // setHSBPixel
