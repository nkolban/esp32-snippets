

#include "WS2812.h"
#include <esp_log.h>
#include <driver/gpio.h>
#include <stdint.h>
#include <driver/rmt.h>
#include <stdlib.h>
#include <string.h>
#include "sdkconfig.h"

static char tag[] = "WS2812";

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
static void setItem1(rmt_item32_t *pItem) {
	pItem->level0 = 1;
	pItem->duration0 = 10;
	pItem->level1 = 0;
	pItem->duration1 = 6;
} // setItem1



/**
 * Set two levels of RMT output to the Neopixel value for a "0".
 * This is:
 * a logic 1 for 0.35us
 * a logic 0 for 0.8us
 */
static void setItem0(rmt_item32_t *pItem) {
	pItem->level0 = 1;
	pItem->duration0 = 4;
	pItem->level1 = 0;
	pItem->duration1 = 8;
} // setItem0

/*
 * Internal function not exposed.  Get the pixel channel color from the channel
 * type which should be one of 'R', 'G' or 'B'.
 */
static uint8_t getChannelValueByType(char type, pixel_t pixel) {
	switch(type) {
		case 'r':
		case 'R':
			return pixel.red;
		case 'b':
		case 'B':
			return pixel.blue;
		case 'g':
		case 'G':
			return pixel.green;
	}
	ESP_LOGW(tag, "Unknown color channel 0x%2x", type);
	return 0;
} // getChannelValueByType



/**
 * Construct a wrapper for the pixels.
 * @param [in] channel The RMT channel to use.  Typically 0.
 * @param [in] gpioNum The GPIO pin used to drive the data.
 * @param [in] pixelCount The number of pixels in the strand.
 */
WS2812::WS2812(int channel, int gpioNum, uint16_t pixelCount) {
	this->pixelCount = pixelCount;
	this->channel = (rmt_channel_t)channel;
	this->items = (rmt_item32_t *)calloc(sizeof(rmt_item32_t), pixelCount * 24);
	this->pixels = (pixel_t *)calloc(sizeof(pixel_t),pixelCount);
	this->colorOrder = (char *)"RGB";
	clear();

	rmt_config_t config;
	config.rmt_mode = RMT_MODE_TX;
	config.channel = this->channel;
	config.gpio_num = (gpio_num_t)gpioNum;
	config.mem_block_num = 1;
	config.tx_config.loop_en = 0;
	config.tx_config.carrier_en = 0;
	config.tx_config.idle_output_en = 1;
	config.tx_config.idle_level = (rmt_idle_level_t)0;
	config.tx_config.carrier_duty_percent = 50;
	config.tx_config.carrier_freq_hz = 10000;
	config.tx_config.carrier_level = (rmt_carrier_level_t)1;
	config.clk_div = 8;

	ESP_ERROR_CHECK(rmt_config(&config));
	ESP_ERROR_CHECK(rmt_driver_install(this->channel, 0, 0));
} // WS2812


/**
 * Show the current Neopixel data.
 *
 * We loop through our array of pixels.  For each pixel we have to add 24
 * bits of output.  8 bits for red, 8 bits for green and 8 bits for blue.
 * Each bit of neopixel data is two levels of RMT which is one RMT item.
 * We determine the bit value of the neopixel and then call either
 * setItem1() or setItem0() which sets the corresponding 2 levels of output
 * on the next RMT item.
 */
void WS2812::show() {
	uint32_t i,j;
	rmt_item32_t *pCurrentItem = this->items;
	for (i=0; i<this->pixelCount; i++) {
		uint32_t currentPixel = getChannelValueByType(this->colorOrder[0], this->pixels[i]) |
				(getChannelValueByType(this->colorOrder[1], this->pixels[i]) << 8) |
				(getChannelValueByType(this->colorOrder[2], this->pixels[i]) << 16);
		ESP_LOGD(tag, "Pixel value: %x", currentPixel);
		for (j=0; j<24; j++) {
			if (currentPixel & (1<<(23-j))) {
				setItem1(pCurrentItem);
			} else {
				setItem0(pCurrentItem);
			}
			pCurrentItem++;
		}
	}
	// Show the pixels.
	ESP_ERROR_CHECK(rmt_write_items(this->channel, this->items, this->pixelCount*24, 1 /* wait till done */));
} // show


/*
 * Set the color order of data sent to the LEDs.
 * The default is "RGB" but we can specify
 * an alternate order by supply an alternate three character string made up of 'R', 'G' and 'B'
 * for example "GRB".
 */
void WS2812::setColorOrder(char *colorOrder) {
	if (colorOrder != NULL && strlen(colorOrder) == 3) {
		this->colorOrder = colorOrder;
	}
} // setColorOrder


/**
 * Set the given pixel to the specified color.
 * @param [in] index The pixel that is to have its color set.
 * @param [in] red The amount of red in the pixel.
 * @param [in] green The amount of green in the pixel.
 * @param [in] blue The amount of blue in the pixel.
 */
void WS2812::setPixel(uint16_t index, uint8_t red, uint8_t green,	uint8_t blue) {
	if (index >= this->pixelCount) {
		ESP_LOGE(tag, "setPixel: index out of range: %d", index);
		return;
	}
	this->pixels[index].red = red;
	this->pixels[index].green = green;
	this->pixels[index].blue = blue;
} // setPixel

/**
 * Set the given pixel to the specified color.
 * @param [in] index The pixel that is to have its color set.
 * @param [in] pixel The color value of the pixel.
 */
void WS2812::setPixel(uint16_t index, pixel_t pixel) {
	if (index >= this->pixelCount) {
		ESP_LOGE(tag, "setPixel: index out of range: %d", index);
		return;
	}
	this->pixels[index] = pixel;
} // setPixel


/**
 * Set the given pixel to the specified color.
 * @param [in] index The pixel that is to have its color set.
 * @param [in] pixel The color value of the pixel.
 */
void WS2812::setPixel(uint16_t index, uint32_t pixel) {
	if (index >= this->pixelCount) {
		ESP_LOGE(tag, "setPixel: index out of range: %d", index);
		return;
	}
	this->pixels[index].red = pixel & 0xff;
	this->pixels[index].green = (pixel & 0xff00) >> 8;
	this->pixels[index].blue = (pixel & 0xff0000) >> 16;
} // setPixel


/**
 * Clear all the pixel colors.
 * This sets all the pixels to off.
 */
void WS2812::clear() {
	uint16_t i;
	for (i=0; i<this->pixelCount; i++) {
		this->pixels[i].red = 0;
		this->pixels[i].green = 0;
		this->pixels[i].blue = 0;
	}
} // clear

WS2812::~WS2812() {
	free(this->items);
	free(this->pixels);
} // ~WS2812()
