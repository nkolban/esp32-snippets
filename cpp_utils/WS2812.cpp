#include <esp_log.h>
#include <driver/rmt.h>
#include <driver/gpio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>

#include "GPIO.h"
#include "sdkconfig.h"
#include "WS2812.h"

#if CONFIG_CXX_EXCEPTIONS != 1
#error "C++ exception handling must be enabled within make menuconfig. See Compiler Options > Enable C++ Exceptions."
#endif


static const char* LOG_TAG = "WS2812";

/**
 * A NeoPixel is defined by 3 bytes ... red, green and blue.
 * Each byte is composed of 8 bits ... therefore a NeoPixel is 24 bits of data.
 * At the underlying level, 1 bit of NeoPixel data is one item (two levels)
 * This means that the number of items we need is:
 *
 * #pixels * 24
 *
 */

/**
 * Set two levels of RMT output to the Neopixel value for a "1".
 * This is:
 * a logic 1 for 0.7us
 * a logic 0 for 0.6us
 */
static void setItem1(rmt_item32_t* pItem) {
	assert(pItem != nullptr);
	pItem->level0    = 1;
	pItem->duration0 = 10;
	pItem->level1    = 0;
	pItem->duration1 = 6;
} // setItem1



/**
 * Set two levels of RMT output to the Neopixel value for a "0".
 * This is:
 * a logic 1 for 0.35us
 * a logic 0 for 0.8us
 */
static void setItem0(rmt_item32_t* pItem) {
	assert(pItem != nullptr);
	pItem->level0    = 1;
	pItem->duration0 = 4;
	pItem->level1    = 0;
	pItem->duration1 = 8;
} // setItem0


/**
 * Add an RMT terminator into the RMT data.
 */
static void setTerminator(rmt_item32_t* pItem) {
	assert(pItem != nullptr);
	pItem->level0    = 0;
	pItem->duration0 = 0;
	pItem->level1    = 0;
	pItem->duration1 = 0;
} // setTerminator

/*
 * Internal function not exposed.  Get the pixel channel color from the channel
 * type which should be one of 'R', 'G' or 'B'.
 */
static uint8_t getChannelValueByType(char type, pixel_t pixel) {
	switch (type) {
		case 'r':
		case 'R':
			return pixel.red;
		case 'b':
		case 'B':
			return pixel.blue;
		case 'g':
		case 'G':
			return pixel.green;
		default:
			ESP_LOGW(LOG_TAG, "Unknown color channel 0x%2x", type);
			return 0;
	}
} // getChannelValueByType


/**
 * @brief Construct a wrapper for the pixels.
 *
 * In order to drive the NeoPixels we need to supply some basic information.  This
 * includes the GPIO pin that is connected to the data-in (DIN) of the devices.
 * Since we also want to be able to drive a string of pixels, we need to tell the class
 * how many pixels are present in the string.
 *

 * @param [in] dinPin The GPIO pin used to drive the data.
 * @param [in] pixelCount The number of pixels in the strand.
 * @param [in] channel The RMT channel to use.  Defaults to RMT_CHANNEL_0.
 */
WS2812::WS2812(gpio_num_t dinPin, uint16_t pixelCount, int channel) {
	/*
	if (pixelCount == 0) {
		throw std::range_error("Pixel count was 0");
	}
	*/
	assert(ESP32CPP::GPIO::inRange(dinPin));

	this->pixelCount = pixelCount;
	this->channel    = (rmt_channel_t) channel;

	// The number of items is number of pixels * 24 bits per pixel + the terminator.
	// Remember that an item is TWO RMT output bits ... for NeoPixels this is correct because
	// on Neopixel bit is TWO bits of output ... the high value and the low value

	this->items      = new rmt_item32_t[pixelCount * 24 + 1];
	this->pixels     = new pixel_t[pixelCount];
	this->colorOrder = (char*) "GRB";
	clear();

	rmt_config_t config;
	config.rmt_mode                  = RMT_MODE_TX;
	config.channel                   = this->channel;
	config.gpio_num                  = dinPin;
	config.mem_block_num             = 8 - this->channel;
	config.clk_div                   = 8;
	config.tx_config.loop_en         = 0;
	config.tx_config.carrier_en      = 0;
	config.tx_config.idle_output_en  = 1;
	config.tx_config.idle_level      = (rmt_idle_level_t) 0;
	config.tx_config.carrier_freq_hz = 10000;
	config.tx_config.carrier_level   = (rmt_carrier_level_t)1;
	config.tx_config.carrier_duty_percent = 50;

	ESP_ERROR_CHECK(rmt_config(&config));
	ESP_ERROR_CHECK(rmt_driver_install(this->channel, 0, 0));
} // WS2812


/**
 * @brief Show the current Neopixel data.
 *
 * Drive the LEDs with the values that were previously set.
 */
void WS2812::show() {
	auto pCurrentItem = this->items;

	for (uint16_t i = 0; i < this->pixelCount; i++) {
		uint32_t currentPixel =
				(getChannelValueByType(this->colorOrder[0], this->pixels[i]) << 16) |
				(getChannelValueByType(this->colorOrder[1], this->pixels[i]) << 8)  |
				(getChannelValueByType(this->colorOrder[2], this->pixels[i]));

		ESP_LOGD(LOG_TAG, "Pixel value: %x", currentPixel);
		for (int8_t j = 23; j >= 0; j--) {
			// We have 24 bits of data representing the red, green amd blue channels. The value of the
			// 24 bits to output is in the variable current_pixel.  We now need to stream this value
			// through RMT in most significant bit first.  To do this, we iterate through each of the 24
			// bits from MSB to LSB.
			if (currentPixel & (1 << j)) {
				setItem1(pCurrentItem);
			} else {
				setItem0(pCurrentItem);
			}
			pCurrentItem++;
		}
	}
	setTerminator(pCurrentItem); // Write the RMT terminator.

	// Show the pixels.
	ESP_ERROR_CHECK(rmt_write_items(this->channel, this->items, this->pixelCount * 24, 1 /* wait till done */));
} // show


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
void WS2812::setColorOrder(char* colorOrder) {
	if (colorOrder != nullptr && strlen(colorOrder) == 3) {
		this->colorOrder = colorOrder;
	}
} // setColorOrder


/**
 * @brief Set the given pixel to the specified color.
 *
 * The LEDs are not actually updated until a call to show().
 *
 * @param [in] index The pixel that is to have its color set.
 * @param [in] red The amount of red in the pixel.
 * @param [in] green The amount of green in the pixel.
 * @param [in] blue The amount of blue in the pixel.
 */
void WS2812::setPixel(uint16_t index, uint8_t red, uint8_t green, uint8_t blue) {
	assert(index < pixelCount);
	this->pixels[index].red   = red;
	this->pixels[index].green = green;
	this->pixels[index].blue  = blue;
} // setPixel


/**
 * @brief Set the given pixel to the specified color.
 *
 * The LEDs are not actually updated until a call to show().
 *
 * @param [in] index The pixel that is to have its color set.
 * @param [in] pixel The color value of the pixel.
 */
void WS2812::setPixel(uint16_t index, pixel_t pixel) {
	assert(index < pixelCount);
	this->pixels[index] = pixel;
} // setPixel


/**
 * @brief Set the given pixel to the specified color.
 *
 * The LEDs are not actually updated until a call to show().
 *
 * @param [in] index The pixel that is to have its color set.
 * @param [in] pixel The color value of the pixel.
 */
void WS2812::setPixel(uint16_t index, uint32_t pixel) {
	assert(index < pixelCount);
	this->pixels[index].red   = pixel & 0xff;
	this->pixels[index].green = (pixel & 0xff00) >> 8;
	this->pixels[index].blue  = (pixel & 0xff0000) >> 16;
} // setPixel

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
void WS2812::setHSBPixel(uint16_t index, uint16_t hue, uint8_t saturation, uint8_t brightness) {
	double sat_red;
	double sat_green;
	double sat_blue;
	double ctmp_red;
	double ctmp_green;
	double ctmp_blue;
	double new_red;
	double new_green;
	double new_blue;
	double dSaturation = (double) saturation / 255;
	double dBrightness = (double) brightness / 255;

	assert(index < pixelCount);

	if (hue < 120) {
		sat_red = (120 - hue) / 60.0;
		sat_green = hue / 60.0;
		sat_blue = 0;
	} else if (hue < 240) {
		sat_red = 0;
		sat_green = (240 - hue) / 60.0;
		sat_blue = (hue - 120) / 60.0;
	} else {
		sat_red = (hue - 240) / 60.0;
		sat_green = 0;
		sat_blue = (360 - hue) / 60.0;
	}

	if (sat_red > 1.0) {
		sat_red = 1.0;
	}
	if (sat_green > 1.0) {
		sat_green = 1.0;
	}
	if (sat_blue > 1.0) {
		sat_blue = 1.0;
	}

	ctmp_red = 2 * dSaturation * sat_red + (1 - dSaturation);
	ctmp_green = 2 * dSaturation * sat_green + (1 - dSaturation);
	ctmp_blue = 2 * dSaturation * sat_blue + (1 - dSaturation);

	if (dBrightness < 0.5) {
		new_red = dBrightness * ctmp_red;
		new_green = dBrightness * ctmp_green;
		new_blue = dBrightness * ctmp_blue;
	} else {
		new_red = (1 - dBrightness) * ctmp_red + 2 * dBrightness - 1;
		new_green = (1 - dBrightness) * ctmp_green + 2 * dBrightness - 1;
		new_blue = (1 - dBrightness) * ctmp_blue + 2 * dBrightness - 1;
	}

	this->pixels[index].red   = (uint8_t)(new_red * 255);
	this->pixels[index].green = (uint8_t)(new_green * 255);
	this->pixels[index].blue  = (uint8_t)(new_blue * 255);
} // setHSBPixel


/**
 * @brief Clear all the pixel colors.
 *
 * This sets all the pixels to off which is no brightness for all of the color channels.
 * The LEDs are not actually updated until a call to show().
 */
void WS2812::clear() {
	for (uint16_t i = 0; i < this->pixelCount; i++) {
		this->pixels[i].red   = 0;
		this->pixels[i].green = 0;
		this->pixels[i].blue  = 0;
	}
} // clear


/**
 * @brief Class instance destructor.
 */
WS2812::~WS2812() {
	delete this->items;
	delete this->pixels;
} // ~WS2812()
